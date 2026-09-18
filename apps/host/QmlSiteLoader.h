#pragma once

#include <QObject>
#include <QUrl>
#include <functional>

// One navigation owns one loader. Destroying it cancels all network work.
class QmlSiteLoader final : public QObject
{
public:
    using Completion = std::function<void(QByteArray, QString)>;
    QmlSiteLoader(QObject *parent, QUrl address, QString packageId,
                  QString route, Completion completion);
    void start();
    static bool validateDescriptor(const QByteArray &json, const QUrl &address,
                                   const QString &packageId, const QString &route,
                                   QUrl &archive, QByteArray &sha256);

private:
    void get(const QUrl &url, qint64 limit, bool descriptor);
    void finish(QByteArray bytes, QString error);
    QUrl address_;
    QString packageId_;
    QString route_;
    QByteArray digest_;
    Completion completion_;
};
