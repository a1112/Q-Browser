#include "PilotRoutes.h"
#include "DemoRoutes.h"

#include <array>

std::optional<RouteRegistry> createPilotRouteRegistry(
    const QUrl &mockOrigin,
    const QString &workerAppId)
{
    if (!mockOrigin.isValid() || mockOrigin.isRelative()
        || workerAppId.isEmpty()
        || (mockOrigin.scheme() != QStringLiteral("http")
            && mockOrigin.scheme() != QStringLiteral("https"))) {
        return std::nullopt;
    }
    RouteRegistry routes;
    constexpr std::array workerPatterns{
        "/login", "/dashboard", "/orders", "/orders/:id", "/orders/:id/edit",
        "/customers", "/customers/:id", "/files", "/settings"};
    for (const char *pattern : workerPatterns) {
        const RouteRecord worker{QString::fromLatin1(pattern),
                                 Engine::QmlWorker,
                                 isQmlDemoPackage(workerAppId) ? QStringLiteral("com.qbrowser.pilot") : workerAppId,
                                 QStringLiteral("qml/Main.qml")};
        if (routes.add(worker) != RouteAddResult::Added) {
            return std::nullopt;
        }
    }
    for (const auto &demo : qmlDemoRoutes) {
        if (routes.add({QString::fromLatin1(demo.path), Engine::QmlWorker,
                        QString::fromLatin1(demo.packageId), QStringLiteral("qml/Main.qml")})
            != RouteAddResult::Added) return std::nullopt;
    }
    const RouteRecord web{QStringLiteral("/web/help"),
                          Engine::WebEngine,
                          QStringLiteral("com.qbrowser.web"),
                          mockOrigin.resolved(QUrl(QStringLiteral("help")))
                              .toString(QUrl::FullyEncoded)};
    if (routes.add(web) != RouteAddResult::Added) {
        return std::nullopt;
    }
    return routes;
}
