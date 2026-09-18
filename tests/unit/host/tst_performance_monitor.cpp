#include "PerformanceSampler.h"
#include "PerformancePanel.h"

#include <QApplication>
#include <QComboBox>
#include <QSignalSpy>
#include <QLabel>
#include <QProcess>
#include <QTest>
#include <QThread>
#include <QToolButton>
#include <QTreeWidget>
#include <cstdio>

class PerformanceMonitorTest final : public QObject {
    Q_OBJECT
private slots:
    void computesWholeMachineCpuAndMemory();
    void handlesFirstSampleReusedPidsAndCounterReset();
    void samplesCurrentProcessAndChild();
    void panelPausesStopsAndResumes();
    void reportsUiStalls();
    void tabScopesTrackSharingRemovalAndProcessIdentity();
};

void PerformanceMonitorTest::computesWholeMachineCpuAndMemory()
{
    ProcessCounters root{1, 0, 100, 10000000, 1000, 2000, QStringLiteral("host")};
    ProcessCounters child{2, 1, 200, 5000000, 3000, 4000, QStringLiteral("renderer")};
    const QHash<quint64, ProcessCounters> previous{{1, root}, {2, child}};
    root.cpuTicks += 10000000; // One core-second in a 2-second, 4-core interval.
    child.cpuTicks += 5000000;
    const auto sample = PerformanceSampler::calculate({root, child}, previous, 2000000000, 4);
    QCOMPARE(sample.cpuPercent, 18.75);
    QCOMPARE(sample.processes.at(0).cpuPercent, 12.5);
    QCOMPARE(sample.processes.at(1).cpuPercent, 6.25);
    QCOMPARE(sample.workingSet, quint64(4000));
    QCOMPARE(sample.privateBytes, quint64(6000));
}

void PerformanceMonitorTest::handlesFirstSampleReusedPidsAndCounterReset()
{
    ProcessCounters process{1, 0, 100, 10000000, 1000, 2000, QStringLiteral("host")};
    const auto first = PerformanceSampler::calculate({process}, {}, 0, 4);
    QCOMPARE(first.cpuPercent, -1.0);
    QCOMPARE(first.workingSet, quint64(1000));
    const QHash<quint64, ProcessCounters> previous{{1, process}};
    process.creationTime = 200;
    process.cpuTicks = 900000000;
    QCOMPARE(PerformanceSampler::calculate({process}, previous, 1000000000, 4)
                 .cpuPercent, -1.0);
    QCOMPARE(PerformanceSampler::calculate({process}, previous, 1000000000, 4)
                 .processes.at(0).cpuPercent, -1.0);
    process.creationTime = 100;
    process.cpuTicks = 1;
    QCOMPARE(PerformanceSampler::calculate({process}, previous, 1000000000, 4)
                 .processes.at(0).cpuPercent, -1.0);
    QCOMPARE(PerformanceSampler::calculate({process}, previous, 0, 4).cpuPercent, -1.0);
    QCOMPARE(PerformanceSampler::calculate({process}, previous, 1000000000, 0).cpuPercent, -1.0);
}

void PerformanceMonitorTest::samplesCurrentProcessAndChild()
{
#ifdef Q_OS_WIN
    PerformanceSampler sampler;
    auto sample = sampler.sample();
    QVERIFY2(sample.error.isEmpty(), qPrintable(sample.error));
    QVERIFY(!sample.processes.isEmpty());
    QCOMPARE(sample.processes.first().counters.pid, quint64(QCoreApplication::applicationPid()));
    QVERIFY(sample.workingSet > 0);
    QProcess child;
    child.start(QCoreApplication::applicationFilePath(), {QStringLiteral("--sample-child")});
    QVERIFY(child.waitForStarted());
    const auto childPid = quint64(child.processId());
    QTest::qWait(100);
    sample = sampler.sample();
    QVERIFY(sample.cpuPercent >= 0 && sample.cpuPercent <= 100);
    bool found = false;
    for (const auto &process : sample.processes) {
        if (process.counters.pid == childPid) {
            found = true;
            QCOMPARE(process.counters.parentPid, quint64(QCoreApplication::applicationPid()));
        }
    }
    QVERIFY(found);
    child.write("quit\n");
    child.closeWriteChannel();
    QVERIFY(child.waitForFinished());
    sample = sampler.sample();
    for (const auto &process : sample.processes) QVERIFY(process.counters.pid != childPid);
#else
    QVERIFY(!PerformanceSampler().sample().error.isEmpty());
#endif
}

void PerformanceMonitorTest::panelPausesStopsAndResumes()
{
    PerformancePanel panel([] { return BrowserResourceCounts{3, 2, 1}; });
    panel.resize(320, 800);
    QVERIFY(panel.findChildren<QThread *>().isEmpty());
    panel.show();
    auto *table = panel.findChild<QTreeWidget *>(QStringLiteral("performance-processes"));
    auto *pause = panel.findChild<QToolButton *>(QStringLiteral("performance-pause"));
    auto *counts = panel.findChild<QLabel *>(QStringLiteral("performance-resources"));
    auto *cpu = panel.findChild<QLabel *>(QStringLiteral("performance-cpu"));
    QVERIFY(table && pause && counts && cpu);
    QTRY_VERIFY(counts->text().contains(QStringLiteral("3 个标签页")));
#ifdef Q_OS_WIN
    QTRY_VERIFY(table->topLevelItemCount() > 0);
    QTRY_VERIFY(cpu->text().contains(QLatin1Char('%')));
#endif
    QTest::mouseClick(pause, Qt::LeftButton);
    QVERIFY(panel.findChildren<QThread *>().isEmpty());
    const QString pausedCpu = cpu->text();
    QTest::qWait(1100);
    QCOMPARE(cpu->text(), pausedCpu);
    QTest::mouseClick(pause, Qt::LeftButton);
    QCOMPARE(panel.findChildren<QThread *>().size(), 1);
    panel.hide();
    QVERIFY(panel.findChildren<QThread *>().isEmpty());
    panel.show();
    QCOMPARE(panel.findChildren<QThread *>().size(), 1);
    QTRY_VERIFY(!counts->text().isEmpty());
    const auto screenshot = qEnvironmentVariable("Q_BROWSER_PERFORMANCE_SCREENSHOT");
    if (!screenshot.isEmpty()) {
        QTest::qWait(2200);
        QVERIFY(panel.grab().save(screenshot));
    }
}

void PerformanceMonitorTest::reportsUiStalls()
{
    PerformancePanel panel([] { return BrowserResourceCounts{}; });
    panel.show();
    auto *latency = panel.findChild<QLabel *>(QStringLiteral("performance-latency"));
    auto *status = panel.findChild<QLabel *>(QStringLiteral("performance-status"));
    QVERIFY(latency && status);
    QTRY_VERIFY(status->text().startsWith(QStringLiteral("更新于")));
    // Deliberately block the UI while the independent sampler keeps running.
    QThread::msleep(400);
    QTRY_VERIFY(latency->text().split(QLatin1Char(' '), Qt::SkipEmptyParts)
                    .value(1).toLongLong() >= 200);
}

void PerformanceMonitorTest::tabScopesTrackSharingRemovalAndProcessIdentity()
{
    BrowserResourceCounts resources;
    const auto pid = quint64(QCoreApplication::applicationPid());
    resources.tabResources = {
        {QStringLiteral("one"), QStringLiteral("音乐"), QStringLiteral("app://pilot/demos/elisa"),
         QStringLiteral("前台"), QStringLiteral("独立 Worker"), pid, 0, true},
        {QStringLiteral("two"), QStringLiteral("社区"), QStringLiteral("app://pilot/demos/tokodon"),
         QStringLiteral("后台"), QStringLiteral("渲染进程"), pid, 0, false},
        {QStringLiteral("host"), QStringLiteral("示例中心"), {}, {}, QStringLiteral("Host 共享界面"), 0, 0, false}
    };
    PerformancePanel panel([&resources] { return resources; });
    panel.show();
    auto *scope = panel.findChild<QComboBox *>(QStringLiteral("performance-scope"));
    auto *tabs = panel.findChild<QTreeWidget *>(QStringLiteral("performance-tabs"));
    auto *memory = panel.findChild<QLabel *>(QStringLiteral("performance-memory"));
    auto *attribution = panel.findChild<QLabel *>(QStringLiteral("performance-attribution"));
    QVERIFY(scope && tabs && memory && attribution);
    QTRY_COMPARE(tabs->topLevelItemCount(), 3);
    QSignalSpy removedChoices(scope->model(), &QAbstractItemModel::rowsRemoved);
    QTest::qWait(1200);
    QCOMPARE(removedChoices.count(), 0); // Sampling must not reset an open scope chooser.
    QVERIFY(tabs->topLevelItem(0)->text(0).contains(QStringLiteral("共享进程")));
    scope->setCurrentIndex(scope->findData(QStringLiteral("one")));
    QTRY_VERIFY(attribution->text().startsWith(QStringLiteral("音乐")));
#ifdef Q_OS_WIN
    QTRY_VERIFY(memory->text().contains(QStringLiteral("MB")));
#endif
    // A stale process snapshot must never be attributed to a reused Worker PID.
    resources.tabResources[0].creationTime = 1;
    QTRY_COMPARE(memory->text(), QStringLiteral("内存  —"));
    resources.tabResources.removeFirst();
    resources.tabResources[0].active = true;
    QTRY_COMPARE(tabs->topLevelItemCount(), 2);
    QTRY_COMPARE(scope->currentData().toString(), QStringLiteral("active"));
    QTRY_VERIFY(attribution->text().startsWith(QStringLiteral("社区")));
    resources.tabResources[0].active = false;
    resources.tabResources[1].active = true;
    QTRY_VERIFY(attribution->text().startsWith(QStringLiteral("示例中心")));
    QCOMPARE(memory->text(), QStringLiteral("内存  —"));
}

int main(int argc, char **argv)
{
    if (argc > 1 && QByteArray(argv[1]) == "--sample-child") {
        char buffer[16]{};
        return std::fgets(buffer, sizeof(buffer), stdin) ? 0 : 1;
    }
    QApplication app(argc, argv);
    PerformanceMonitorTest test;
    return QTest::qExec(&test, argc, argv);
}
#include "tst_performance_monitor.moc"
