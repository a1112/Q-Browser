#include "WorkerApplication.h"

#include <QGuiApplication>
#include <QQuickStyle>

int main(int argc, char *argv[])
{
    QGuiApplication application(argc, argv);
    QQuickStyle::setStyle(QStringLiteral("Basic"));
    QCoreApplication::setApplicationName(QStringLiteral("qbrowser-worker"));
    WorkerApplication worker;
    if (!worker.start(QCoreApplication::arguments())) {
        return WorkerApplication::invalidLaunchExitCode();
    }
    return application.exec();
}
