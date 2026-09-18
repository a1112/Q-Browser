#include "AudioBroker.h"
#include "WindowsStableIo.h"
#include <QAudioOutput>
#include <QAudioDevice>
#include <QMediaDevices>
#include <QMediaPlayer>
#include <QDir>
#include <QJsonDocument>
#include <QRegularExpression>
#include <cmath>

// Installed assets legitimately have read-only AppContainer ACEs. They must
// not use the Host-only ACL validator. The caller holds the verified package
// lease; pin both directory membership and exact file identity while reading.
class VerifiedAudioAsset {
public:
    static std::shared_ptr<VerifiedAudioAsset> open(const QString &root, const QString &relative) {
        auto asset = std::make_shared<VerifiedAudioAsset>();
        if (!asset->tree_.openSharedRoot(root)
            || !asset->tree_.addImmutableDirectory(root + QStringLiteral("/assets"))
            || !asset->tree_.addImmutableDirectory(root + QStringLiteral("/assets/audio"))
            || !asset->file_.openReadLocked(root + u'/' + relative, asset->tree_)
            || !asset->file_.hasSingleLink()) return {};
        return asset;
    }
    bool readBounded(quint64 limit, QByteArray &bytes) {
        return revalidate() && file_.readBounded(limit, bytes);
    }
    bool revalidate() const {
        return file_.isSameIdentityAt(file_.path()) && file_.isStableWithin(tree_);
    }
private:
    qbrowser_archive_detail::WindowsStableDirectoryTree tree_;
    qbrowser_archive_detail::WindowsStableFile file_;
};

namespace {
constexpr quint64 MaximumAudioBytes = 8 * 1024 * 1024;
BrokerResult invalid() {
    return BrokerResult::failure(QStringLiteral("audio.invalid_request"),
                                 QStringLiteral("Invalid audio request."));
}
}

AudioBroker::AudioBroker(QString appIdentity, const QString &packageDirectory)
    : identity_(std::move(appIdentity)), root_(QDir(packageDirectory).canonicalPath()),
      player_(std::make_unique<QMediaPlayer>()), output_(std::make_unique<QAudioOutput>())
{
    player_->setAudioOutput(output_.get());
    output_->setVolume(0.35f);
    connect(player_.get(), &QMediaPlayer::errorOccurred, this,
            [this](QMediaPlayer::Error, const QString &) {
        error_ = QStringLiteral("音频无法播放，请检查音频文件与输出设备。");
    });
    if (packageDirectory.isEmpty() || root_.isEmpty()) return;
    const auto file = VerifiedAudioAsset::open(root_, QStringLiteral("assets/audio/catalog.json"));
    QByteArray bytes;
    if (!file || !file->readBounded(64 * 1024, bytes)) return;
    const auto document = QJsonDocument::fromJson(bytes);
    if (!document.isArray() || document.array().size() > 64) return;
    const QRegularExpression identifier(QStringLiteral("^[a-z0-9][a-z0-9-]{0,63}$"));
    for (const auto &value : document.array()) {
        if (!value.isObject()) { tracks_.clear(); catalog_ = {}; return; }
        const auto entry = value.toObject();
        const QString id = entry.value(QStringLiteral("id")).toString();
        if (!identifier.match(id).hasMatch() || tracks_.contains(id)) {
            tracks_.clear(); catalog_ = {}; return;
        }
        // No paths are accepted in IPC or even in the package catalog.
        const auto track = VerifiedAudioAsset::open(
            root_, QStringLiteral("assets/audio/") + id + QStringLiteral(".wav"));
        if (!track) { tracks_.clear(); catalog_ = {}; return; }
        tracks_.insert(id, track);
        QJsonObject safeEntry{{QStringLiteral("id"), id}};
        for (const auto &field : {QStringLiteral("title"), QStringLiteral("artist"), QStringLiteral("album")})
            safeEntry.insert(field, entry.value(field).toString().left(128));
        catalog_.append(safeEntry);
    }
}

AudioBroker::~AudioBroker() { shutdown(); player_.reset(); }
void AudioBroker::shutdown()
{
    if (stopped_) return;
    stopped_ = true;
    player_->stop();
    player_->setSource(QUrl());
    buffer_.close();
    buffer_.setData(QByteArray());
    tracks_.clear();
}

QJsonObject AudioBroker::status() const
{
    const QString state = player_->playbackState() == QMediaPlayer::PlayingState
        ? QStringLiteral("playing") : player_->playbackState() == QMediaPlayer::PausedState
        ? QStringLiteral("paused") : QStringLiteral("stopped");
    return {{QStringLiteral("trackId"), currentId_}, {QStringLiteral("state"), state},
            {QStringLiteral("position"), player_->position()},
            {QStringLiteral("duration"), player_->duration()},
            {QStringLiteral("volume"), output_->volume()},
            {QStringLiteral("ended"), player_->mediaStatus() == QMediaPlayer::EndOfMedia},
            {QStringLiteral("error"), error_}};
}

BrokerResult AudioBroker::invoke(const QString &operation, const QJsonObject &payload,
                                const HostRequestContext &context)
{
    if (stopped_ || context.appIdentity != identity_)
        return BrokerResult::failure(QStringLiteral("capability.denied"), QStringLiteral("Audio authority expired."));
    if (operation == QStringLiteral("catalog")) {
        if (!payload.isEmpty()) return invalid();
        if (catalog_.isEmpty()) return BrokerResult::failure(QStringLiteral("audio.catalog_unavailable"), QStringLiteral("随包音频不可用。"));
        return BrokerResult::success({{QStringLiteral("tracks"), catalog_}});
    }
    if (operation == QStringLiteral("play")) {
        if (payload.size() != 1 || !payload.value(QStringLiteral("trackId")).isString()) return invalid();
        const auto id = payload.value(QStringLiteral("trackId")).toString();
        const auto found = tracks_.constFind(id);
        if (found == tracks_.cend()) return invalid();
        if (QMediaDevices::defaultAudioOutput().isNull())
            return BrokerResult::failure(QStringLiteral("audio.no_device"), QStringLiteral("未找到音频输出设备。"));
        error_.clear();
        if (currentId_ != id || !buffer_.isOpen()) {
            QByteArray bytes;
            if (!(*found)->revalidate() || !(*found)->readBounded(MaximumAudioBytes, bytes)
                || bytes.size() < 44 || !bytes.startsWith("RIFF") || bytes.mid(8, 4) != "WAVE")
                return BrokerResult::failure(QStringLiteral("audio.asset_unavailable"), QStringLiteral("音频资源无法读取。"));
            player_->stop();
            player_->setSource(QUrl());
            buffer_.close();
            buffer_.setData(bytes);
            buffer_.open(QIODevice::ReadOnly);
            currentId_ = id;
            player_->setSourceDevice(&buffer_, QUrl(QStringLiteral("qbrowser-audio.wav")));
        }
        if (player_->mediaStatus() == QMediaPlayer::EndOfMedia) player_->setPosition(0);
        player_->play();
    } else if (operation == QStringLiteral("seek") || operation == QStringLiteral("setVolume")) {
        const QString key = operation == QStringLiteral("seek") ? QStringLiteral("position") : QStringLiteral("volume");
        const auto value = payload.value(key);
        if (payload.size() != 1 || !value.isDouble() || !std::isfinite(value.toDouble())) return invalid();
        const double number = value.toDouble();
        if (key == QStringLiteral("position")) {
            if (number < 0 || number > double(player_->duration()) || !player_->isSeekable()) return invalid();
            player_->setPosition(qint64(number));
        } else {
            if (number < 0 || number > 1) return invalid();
            output_->setVolume(float(number));
        }
    } else {
        if (!payload.isEmpty()) return invalid();
        if (operation == QStringLiteral("pause")) player_->pause();
        else if (operation == QStringLiteral("stop")) player_->stop();
        else if (operation != QStringLiteral("status")) return invalid();
    }
    return BrokerResult::success(status());
}
