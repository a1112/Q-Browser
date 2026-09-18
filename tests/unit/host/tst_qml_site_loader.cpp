#include "QmlSiteLoader.h"
#include <QCryptographicHash>
#include <QJsonDocument>
#include <QJsonObject>
#include <QTcpServer>
#include <QTcpSocket>
#include <QTest>
#include <memory>

class QmlSiteLoaderTest final : public QObject
{
    Q_OBJECT
private slots:
    void rejectsRedirectsAndCancels()
    {
        QTcpServer server;
        QVERIFY(server.listen(QHostAddress::LocalHost));
        bool replied = false;
        connect(&server, &QTcpServer::newConnection, &server, [&] {
            auto *socket = server.nextPendingConnection();
            connect(socket, &QTcpSocket::readyRead, socket, [socket] {
                socket->readAll();
                socket->write("HTTP/1.1 302 Found\r\nLocation: http://example.test/\r\nContent-Length: 0\r\nConnection: close\r\n\r\n");
                socket->disconnectFromHost();
            });
            connect(socket, &QTcpSocket::disconnected, socket, &QObject::deleteLater);
        });
        const QUrl address(QStringLiteral("http://127.0.0.1:%1/demos/elisa").arg(server.serverPort()));
        auto *loader = new QmlSiteLoader(this, address, "com.qbrowser.demo.elisa", "/demos/elisa",
            [&](QByteArray bytes, QString error) { QVERIFY(bytes.isEmpty()); QVERIFY(!error.isEmpty()); replied = true; });
        loader->start();
        QTRY_VERIFY(replied);
        bool staleReply = false;
        auto *cancelled = new QmlSiteLoader(this, address, "com.qbrowser.demo.elisa", "/demos/elisa",
            [&](QByteArray, QString) { staleReply = true; });
        cancelled->start();
        delete cancelled;
        QTest::qWait(100);
        QVERIFY(!staleReply);
    }
    void descriptorBinding()
    {
        const QUrl site(QStringLiteral("http://127.0.0.1:18880/demos/elisa"));
        QJsonObject value{{"schemaVersion", 1}, {"appId", "com.qbrowser.demo.elisa"},
                          {"route", "/demos/elisa"}, {"packageUrl", "/packages/elisa.qapkg"},
                          {"sha256", QString(64, u'a')}};
        QUrl archive;
        QByteArray digest;
        auto valid = [&] { return QmlSiteLoader::validateDescriptor(QJsonDocument(value).toJson(),
            site, QStringLiteral("com.qbrowser.demo.elisa"), QStringLiteral("/demos/elisa"), archive, digest); };
        QVERIFY(valid());
        QCOMPARE(archive.toString(), QStringLiteral("http://127.0.0.1:18880/packages/elisa.qapkg"));
        for (const QString &path : {QStringLiteral("https://evil.test/a.qapkg"),
                                   QStringLiteral("//evil.test/a.qapkg"),
                                   QStringLiteral("/packages/../a.qapkg"),
                                   QStringLiteral("/packages/%2e%2e.qapkg")}) {
            value["packageUrl"] = path;
            QVERIFY(!valid());
        }
        value["packageUrl"] = "/packages/elisa.qapkg";
        value["appId"] = "com.qbrowser.demo.coffee";
        QVERIFY(!valid());
        value["appId"] = "com.qbrowser.demo.elisa";
        value["route"] = "/demos/coffee";
        QVERIFY(!valid());
    }
    void downloadsAndRejectsTampering_data()
    {
        QTest::addColumn<bool>("tamper");
        QTest::newRow("valid") << false;
        QTest::newRow("tamper") << true;
    }
    void downloadsAndRejectsTampering()
    {
        QFETCH(bool, tamper);
        QTcpServer server;
        QVERIFY(server.listen(QHostAddress::LocalHost));
        const QByteArray package("signed-archive-fixture");
        const QJsonObject descriptor{{"schemaVersion", 1}, {"appId", "com.qbrowser.demo.elisa"},
            {"route", "/demos/elisa"}, {"packageUrl", "/packages/elisa.qapkg"},
            {"sha256", QString::fromLatin1(QCryptographicHash::hash(package, QCryptographicHash::Sha256).toHex())}};
        int requests = 0;
        connect(&server, &QTcpServer::newConnection, &server, [&] {
            auto *socket = server.nextPendingConnection();
            auto request = std::make_shared<QByteArray>();
            connect(socket, &QTcpSocket::readyRead, socket, [&, socket, request] {
                request->append(socket->readAll());
                if (!request->contains("\r\n\r\n")) return;
                ++requests;
                QByteArray body;
                if (request->startsWith("GET /demos/elisa ")) {
                    QVERIFY(request->contains("application/vnd.qbrowser.site+json"));
                    body = QJsonDocument(descriptor).toJson();
                } else { body = tamper ? QByteArray("tampered") : package; }
                socket->write("HTTP/1.1 200 OK\r\nConnection: close\r\nContent-Length: "
                              + QByteArray::number(body.size()) + "\r\n\r\n" + body);
                socket->disconnectFromHost();
            });
            connect(socket, &QTcpSocket::disconnected, socket, &QObject::deleteLater);
        });
        bool completed = false;
        QByteArray result;
        QString error;
        auto *loader = new QmlSiteLoader(this,
            QUrl(QStringLiteral("http://127.0.0.1:%1/demos/elisa").arg(server.serverPort())),
            QStringLiteral("com.qbrowser.demo.elisa"), QStringLiteral("/demos/elisa"),
            [&](QByteArray bytes, QString failure) { result = bytes; error = failure; completed = true; });
        loader->start();
        QTRY_VERIFY(completed);
        QCOMPARE(requests, 2);
        QCOMPARE(error.isEmpty(), !tamper);
        QCOMPARE(result, tamper ? QByteArray{} : package);
    }
};
QTEST_GUILESS_MAIN(QmlSiteLoaderTest)
#include "tst_qml_site_loader.moc"
