#pragma once
#include <QString>
#include <array>

struct QmlDemoRoute { const char *id; const char *packageId; const char *path; };
inline constexpr std::array<QmlDemoRoute, 3> qmlDemoRoutes{{
    {"elisa", "com.qbrowser.demo.elisa", "/demos/elisa"},
    {"tokodon", "com.qbrowser.demo.tokodon", "/demos/tokodon"},
    {"coffee", "com.qbrowser.demo.coffee", "/demos/coffee"}
}};
inline bool isQmlDemoPackage(const QString &id) {
    for (const auto &demo : qmlDemoRoutes)
        if (id == QLatin1StringView(demo.packageId)) return true;
    return false;
}
inline bool isQmlDemoRoute(const QString &packageId, const QString &path) {
    for (const auto &demo : qmlDemoRoutes)
        if (packageId == QLatin1StringView(demo.packageId)
            && path == QLatin1StringView(demo.path)) return true;
    return false;
}
