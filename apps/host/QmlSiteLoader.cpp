#include "QmlSiteLoader.h"

#include <QCryptographicHash>
#include <QJsonDocument>
#include <QJsonObject>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QRegularExpression>
#include <QTimer>
#include <memory>
#include <utility>

QmlSiteLoader::QmlSiteLoader(QObject *parent, QUrl address, QString packageId,
                             QString route, Completion completion)
    : QObject(parent), address_(std::move(address)), packageId_(std::move(packageId)),
      route_(std::move(route)), completion_(std::move(completion)) {}

bool QmlSiteLoader::validateDescriptor(const QByteArray &json, const QUrl &address,
                                      const QString &packageId, const QString &route,
                                      QUrl &archive, QByteArray &sha256)
{
    const auto document = QJsonDocument::fromJson(json);
    if (!document.isObject()) return false;
    const auto object = document.object();
    const QString digest = object.value(QStringLiteral("sha256")).toString();
    const QString path = object.value(QStringLiteral("packageUrl")).toString();
    static const QRegularExpression safePath(QStringLiteral("^/packages/[a-zA-Z0-9._-]+\\.qapkg$"));
    static const QRegularExpression safeDigest(QStringLiteral("^[a-f0-9]{64}$"));
    if (object.value(QStringLiteral("schemaVersion")).toInt() != 1
        || object.value(QStringLiteral("appId")).toString() != packageId
        || object.value(QStringLiteral("route")).toString() != route
        || !safePath.match(path).hasMatch() || !safeDigest.match(digest).hasMatch()) return false;
    archive = address.resolved(QUrl(path));
    sha256 = digest.toLatin1();
    return true;
}

void QmlSiteLoader::start() { get(address_, 64 * 1024, true); }

void QmlSiteLoader::finish(QByteArray bytes, QString error)
{
    if (!completion_) return;
    auto callback = std::move(completion_);
    deleteLater();
    callback(std::move(bytes), std::move(error));
}

void QmlSiteLoader::get(const QUrl &url, const qint64 limit, const bool descriptor)
{
    auto *manager = new QNetworkAccessManager(this);
    QNetworkRequest request(url);
    request.setAttribute(QNetworkRequest::RedirectPolicyAttribute, QNetworkRequest::ManualRedirectPolicy);
    request.setAttribute(QNetworkRequest::CookieLoadControlAttribute, QNetworkRequest::Manual);
    request.setAttribute(QNetworkRequest::CookieSaveControlAttribute, QNetworkRequest::Manual);
    request.setRawHeader("Accept", descriptor ? "application/vnd.qbrowser.site+json" : "application/octet-stream");
    request.setTransferTimeout(15000);
    auto *reply = manager->get(request);
    reply->setReadBufferSize(64 * 1024);
    auto data = std::make_shared<QByteArray>();
    auto *deadline = new QTimer(reply);
    deadline->setSingleShot(true);
    connect(deadline, &QTimer::timeout, reply, &QNetworkReply::abort);
    deadline->start(30000);
    connect(reply, &QIODevice::readyRead, this, [reply, data, limit] {
        data->append(reply->readAll());
        if (data->size() > limit) reply->abort();
    });
    connect(reply, &QNetworkReply::finished, this, [this, reply, data, limit, descriptor, manager] {
        data->append(reply->readAll());
        const bool valid = reply->error() == QNetworkReply::NoError && data->size() <= limit
            && reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt() == 200;
        manager->deleteLater();
        if (!valid) {
            finish({}, QStringLiteral("QML 站点请求失败（连接、HTTP 状态、超时或大小限制）。"));
            return;
        }
        if (descriptor) {
            QUrl archive;
            if (!validateDescriptor(*data, address_, packageId_, route_, archive, digest_)) {
                finish({}, QStringLiteral("QML 站点描述无效，或应用与地址不匹配。"));
                return;
            }
            get(archive, 64 * 1024 * 1024, false);
        } else if (QCryptographicHash::hash(*data, QCryptographicHash::Sha256).toHex() != digest_) {
            finish({}, QStringLiteral("QML 站点包摘要校验失败。"));
        } else {
            finish(std::move(*data), {});
        }
    });
}
