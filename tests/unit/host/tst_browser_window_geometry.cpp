#include "BrowserWindowGeometry.h"
#include <QTest>

class BrowserWindowGeometryTest final : public QObject
{
    Q_OBJECT
private slots:
    void compactFirstLaunch();
    void preservesUserGeometry();
    void repairsRemovedMonitor();
    void clampsSmallWorkArea();
    void invalidScreens();
};

void BrowserWindowGeometryTest::compactFirstLaunch()
{
    const QRect screen(100, 50, 1600, 900);
    const QRect result = restoreBrowserWindowGeometry({}, screen, {screen});
    QCOMPARE(result.size(), QSize(1100, 681));
    QCOMPARE(result.center(), screen.center());
}
void BrowserWindowGeometryTest::preservesUserGeometry()
{
    const QRect primary(0, 25, 1440, 850);
    const QRect secondary(-1600, -100, 1600, 950);
    for (const QRect saved : {QRect(0, 25, 1440, 850), QRect(-1500, 50, 1200, 740)})
        QCOMPARE(restoreBrowserWindowGeometry(saved, primary, {primary, secondary}), saved);
}
void BrowserWindowGeometryTest::repairsRemovedMonitor()
{
    const QRect screen(0, 25, 1440, 850);
    const QRect result = restoreBrowserWindowGeometry(QRect(2500, 100, 900, 600), screen, {screen});
    QCOMPARE(result.size(), QSize(900, 600));
    QVERIFY(screen.adjusted(24, 24, -24, -24).contains(result));
}
void BrowserWindowGeometryTest::clampsSmallWorkArea()
{
    const QRect screen(-1024, 30, 1024, 640);
    const QRect result = restoreBrowserWindowGeometry(QRect(-1000, 60, 1500, 900), screen, {screen});
    QVERIFY(screen.adjusted(24, 24, -24, -24).contains(result));
    const QRect first = restoreBrowserWindowGeometry({}, screen, {screen});
    QVERIFY(first.width() < screen.width());
    QVERIFY(first.height() < screen.height());
}
void BrowserWindowGeometryTest::invalidScreens()
{
    QCOMPARE(restoreBrowserWindowGeometry({}, {}, {}), QRect(0, 0, 1100, 720));
    const QRect screen(-800, 0, 800, 600);
    QVERIFY(screen.contains(restoreBrowserWindowGeometry({}, {}, {QRect(), screen})));
}
QTEST_APPLESS_MAIN(BrowserWindowGeometryTest)
#include "tst_browser_window_geometry.moc"
