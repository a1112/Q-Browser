#include "AudioBroker.h"
#include "CapabilityBroker.h"
#include <QDir>
#include <QFile>
#include <QJsonArray>
#include <QTemporaryDir>
#include <QTest>

class AudioBrokerTest final : public QObject {
    Q_OBJECT
private slots:
    void verifiedAssetsAndRealPlayback()
    {
        QTemporaryDir fixture;
        QVERIFY(fixture.isValid());
        const QString audio = fixture.path() + QStringLiteral("/assets/audio");
        QVERIFY(QDir().mkpath(audio));
        for (const auto &name : {"catalog.json", "morning.wav", "rain.wav", "night.wav"})
            QVERIFY(QFile::copy(QStringLiteral(Q_BROWSER_SOURCE_DIR "/packages/elisa/assets/audio/") + QLatin1StringView(name), audio + u'/' + QLatin1StringView(name)));
        AudioBroker broker(QStringLiteral("com.qbrowser.demo.elisa"), fixture.path());
        HostRequestContext context{QStringLiteral("com.qbrowser.demo.elisa"), QStringLiteral("test-audio"), nullptr};
        const auto catalog = broker.invoke(QStringLiteral("catalog"), {}, context);
        QVERIFY(catalog.ok);
        QCOMPARE(catalog.value.value(QStringLiteral("tracks")).toArray().size(), 3);
        auto outsider = context;
        outsider.appIdentity = QStringLiteral("com.qbrowser.demo.tokodon");
        QVERIFY(!broker.invoke(QStringLiteral("catalog"), {}, outsider).ok);
        for (const auto &id : {"../night", "file:///C:/song.wav", "https://example.com/audio.wav", "foreign-track"})
            QVERIFY(!broker.invoke(QStringLiteral("play"), {{QStringLiteral("trackId"), QLatin1StringView(id)}}, context).ok);
        QVERIFY(!broker.invoke(QStringLiteral("play"), {{QStringLiteral("url"), QStringLiteral("morning.wav")}}, context).ok);
        QVERIFY(!broker.invoke(QStringLiteral("setVolume"), {{QStringLiteral("volume"), 2}}, context).ok);
        QCOMPARE(broker.invoke(QStringLiteral("status"), {}, context).value.value(QStringLiteral("state")).toString(), QStringLiteral("stopped"));
        const auto played = broker.invoke(QStringLiteral("play"), {{QStringLiteral("trackId"), QStringLiteral("morning")}}, context);
        if (played.errorCode == QStringLiteral("audio.no_device")) QSKIP("No audio output device; explicit error verified.");
        QVERIFY(played.ok);
        QTRY_VERIFY_WITH_TIMEOUT(broker.invoke(QStringLiteral("status"), {}, context).value.value(QStringLiteral("position")).toInteger() > 100, 10000);
        const auto status = broker.invoke(QStringLiteral("status"), {}, context).value;
        QCOMPARE(status.value(QStringLiteral("duration")).toInteger(), 24000);
        QVERIFY(status.value(QStringLiteral("error")).toString().isEmpty());
        QVERIFY(broker.invoke(QStringLiteral("seek"), {{QStringLiteral("position"), 12000}}, context).ok);
        QTRY_VERIFY(broker.invoke(QStringLiteral("status"), {}, context).value.value(QStringLiteral("position")).toInteger() >= 12000);
        QVERIFY(broker.invoke(QStringLiteral("pause"), {}, context).ok);
        QCOMPARE(broker.invoke(QStringLiteral("status"), {}, context).value.value(QStringLiteral("state")).toString(), QStringLiteral("paused"));
        broker.shutdown();
        QVERIFY(!broker.invoke(QStringLiteral("play"), {{QStringLiteral("trackId"), QStringLiteral("morning")}}, context).ok);
    }
};
QTEST_MAIN(AudioBrokerTest)
#include "tst_audio_broker.moc"
