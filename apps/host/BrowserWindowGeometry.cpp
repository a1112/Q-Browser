#include "BrowserWindowGeometry.h"

#include <QSize>
#include <algorithm>

namespace
{
constexpr int DefaultWindowWidth = 1100;
constexpr int DefaultWindowHeight = 720;

QRect safeArea(const QRect &available)
{
    const int margin = std::min({24, available.width() / 8, available.height() / 8});
    return available.adjusted(margin, margin, -margin, -margin);
}

QRect centeredDefault(const QRect &available)
{
    const QRect safe = safeArea(available);
    QRect result(QPoint(), QSize(
        std::min(DefaultWindowWidth, safe.width() * 4 / 5),
        std::min(DefaultWindowHeight, safe.height() * 4 / 5)));
    result.moveCenter(safe.center());
    return result;
}
}

QRect restoreBrowserWindowGeometry(
    const QRect &persisted,
    const QRect &primary,
    const QList<QRect> &screens)
{
    QRect target;
    qint64 largestOverlap = 0;
    for (const QRect &available : screens) {
        if (!available.isValid()) continue;
        if (persisted.isValid() && available.contains(persisted)) return persisted;
        const QRect overlap = available.intersected(persisted);
        const qint64 area = qint64(overlap.width()) * overlap.height();
        if (area > largestOverlap) { target = available; largestOverlap = area; }
    }
    if (!target.isValid()) {
        target = primary.isValid() ? primary : QRect();
        if (!target.isValid()) {
            const auto first = std::ranges::find_if(screens, [](const QRect &r) { return r.isValid(); });
            if (first != screens.cend()) target = *first;
        }
    }
    if (!target.isValid()) return QRect(0, 0, DefaultWindowWidth, DefaultWindowHeight);
    if (!persisted.isValid()) return centeredDefault(target);

    // Monitor removal or a smaller work area should not discard the user's size.
    const QRect safe = safeArea(target);
    QRect result = persisted;
    result.setSize(result.size().boundedTo(safe.size()));
    result.moveLeft(std::clamp(result.left(), safe.left(), safe.right() - result.width() + 1));
    result.moveTop(std::clamp(result.top(), safe.top(), safe.bottom() - result.height() + 1));
    return result;
}
