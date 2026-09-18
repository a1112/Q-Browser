[CmdletBinding()]
param(
    [string]$QtRoot = 'C:\Qt\6.11.0\msvc2022_64',
    [string]$CMake = 'C:\Qt\Tools\CMake_64\bin\cmake.exe',
    [string]$OpenSslRoot = '',
    [string]$TrustedRoot = '',
    [switch]$SkipBuild,
    [switch]$Rebuild
)
$ErrorActionPreference = 'Stop'
Set-StrictMode -Version Latest
$repo = Split-Path $PSScriptRoot -Parent
if (!$OpenSslRoot) { $OpenSslRoot = Join-Path $repo 'build\dependencies\Tools\OpenSSLv3\Win_x64' }
if (!$TrustedRoot) { $TrustedRoot = Join-Path ($env:SystemDrive + '\') 'QBrowserQmlDemos' }
$deploy = Join-Path $TrustedRoot 'release-deploy'
$state = Join-Path $TrustedRoot 'manual-deployed-smoke'
if ($SkipBuild -and $Rebuild) { throw 'Choose either -SkipBuild or -Rebuild.' }
if (!$SkipBuild) {
    & "$PSScriptRoot\build-release.ps1" -QtRoot $QtRoot -CMake $CMake -OpenSslRoot $OpenSslRoot `
        -TrustedRoot $TrustedRoot -IncludeQmlDemos -DevelopmentOnly -RunAcceptance:$false -Clean:$Rebuild
}
& "$PSScriptRoot\build-release.ps1" -TrustedRoot $TrustedRoot -PrepareManualState $state
if (!(Test-Path "$deploy\packages\com.qbrowser.demo.coffee-1.0.0.qapkg")) {
    throw 'Deployment does not contain the three signed QML demos. Build with -IncludeQmlDemos.'
}
$common = @('--package-mode', '--mock-origin=http://127.0.0.1:18765',
    "--trusted-public-key=$deploy\trust\dev-public.pem", "--package-store=$state\package-store",
    "--sandbox-temp=$state\sandbox-temp", "--runtime-root=$deploy\runtime",
    "--worker-executable=$deploy\runtime\qbrowser-worker.exe", "--telemetry-directory=$state\telemetry",
    "--storage-directory=$state\storage", "--deployment-root=$deploy",
    "--browser-state-directory=$state\browser-state", '--health-window-ms=2000', '--heartbeat-timeout-ms=10000')
function Start-DemoHost([string]$Id, [bool]$Install) {
    $arguments = $common + "--app-id=$Id"
    if ($Install) { $arguments += "--install-package=$deploy\packages\$Id-1.0.0.qapkg" }
    $quoted = ($arguments | ForEach-Object { '"' + $_.Replace('"','\"') + '"' }) -join ' '
    return Start-Process "$deploy\host\qbrowser-host.exe" -ArgumentList $quoted -PassThru `
        -RedirectStandardOutput "$state\$Id.stdout" -RedirectStandardError "$state\$Id.stderr"
}
# Each installation goes through the production Host's authenticated startup,
# package verification, activation journal and immutable package guard.
foreach ($id in @('com.qbrowser.demo.elisa', 'com.qbrowser.demo.tokodon', 'com.qbrowser.demo.coffee', 'com.qbrowser.pilot')) {
    $process = Start-DemoHost $id $true
    $deadline = [Diagnostics.Stopwatch]::StartNew()
    do {
        Start-Sleep -Milliseconds 200
        $process.Refresh()
        if ($process.HasExited) { throw "Package startup failed for $id. See $state\$id.stderr" }
        if ($deadline.ElapsedMilliseconds -gt 60000) { throw "Timed out activating $id" }
    } until ($process.MainWindowHandle -ne 0)
    Start-Sleep -Milliseconds 1000
    if (!$process.CloseMainWindow()) { throw "Could not close installer Host for $id" }
    if (!$process.WaitForExit(30000) -or $process.ExitCode -ne 0) { throw "Host cleanup failed for $id" }
}
$hostProcess = Start-DemoHost 'com.qbrowser.pilot' $false
Write-Output "Demo environment started. PID=$($hostProcess.Id)"
Write-Output "Deployment: $deploy"
Write-Output 'Open QML 应用 in the example gallery. Pilot API routes need a separate mock API; the demos work offline.'
