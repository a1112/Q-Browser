#pragma once
#include "CapabilityBroker.h"
#include <QBuffer>
#include <QJsonArray>
#include <QHash>
#include <QObject>
#include <memory>

class QMediaPlayer;
class QAudioOutput;
class VerifiedAudioAsset;

// One instance per admitted Worker incarnation. Only a verified package
// directory supplied by the Host can provide the catalog and audio bytes.
class AudioBroker final : public QObject, public CapabilityService {
public:
    AudioBroker(QString appIdentity, const QString &packageDirectory);
    ~AudioBroker() override;
    BrokerResult invoke(const QString &operation, const QJsonObject &payload,
                        const HostRequestContext &context) override;
    void shutdown();
private:
    QJsonObject status() const;
    QString identity_;
    QString root_;
    QString currentId_;
    QString error_;
    QJsonArray catalog_;
    QHash<QString, std::shared_ptr<VerifiedAudioAsset>> tracks_;
    std::unique_ptr<QMediaPlayer> player_;
    std::unique_ptr<QAudioOutput> output_;
    QBuffer buffer_;
    bool stopped_ = false;
};
