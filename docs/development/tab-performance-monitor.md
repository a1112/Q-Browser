# Per-tab performance monitoring

Open the **性能** panel and choose **当前标签（跟随切换）** or a named
tab in **监控范围**. The CPU, working set, private commit, process list and
60-sample charts then describe that selection. **全部进程** retains the
browser-wide totals. The tab table shows title, lifecycle, PID, CPU and memory;
double-clicking a row selects its trend. Hover over a title for its route and
the attribution details.

QML tabs use their own authenticated Worker's stable process handle to obtain
PID and creation time. Samples with a mismatched creation time are rejected.
Closing a tab removes it from the selector; a removed selection falls back to
following the active tab. Switching tabs or Worker incarnations clears the old
chart. Paused monitoring does not retain another scope's metrics after a scope
change. Sampling still stops when the panel is hidden.

Web tabs report the main-frame renderer identified by QWebEnginePage. Multiple
tabs with the same renderer PID are explicitly marked as sharing that process;
its resource use is not divided or represented as exclusive per-page usage.
Out-of-process subframes and shared browser services are not attributed to an
individual tab. Host/Widgets tabs and the Host audio proxy share the Host
process, so their allocations cannot be separately measured here. Unavailable
tab metrics are displayed as a dash, not zero. Global totals include the shared
Host and process tree, and are not calculated by summing the tab rows.

Validation: `tst_performance_monitor` covers CPU normalization, PID reuse,
process exit, pause/hide cleanup, UI stalls, per-tab selection, sharing,
closed-tab fallback and stable selector contents across samples.
