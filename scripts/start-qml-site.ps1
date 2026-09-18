param(
    [string]$Python = 'E:\Python310\python.exe',
    [string]$Packages = 'C:\QBrowserQmlDemos\release-deploy\packages',
    [int]$Port = 18880,
    [string]$Bind = '127.0.0.1'
)
$ErrorActionPreference = 'Stop'
$serverPath = Join-Path $PSScriptRoot '..\sites\qml-demo-site\server.py'
if (-not (Test-Path -LiteralPath $Packages -PathType Container)) {
    throw '请先使用 start-qml-demos.ps1 准备签名示例包。'
}
& $Python $serverPath --packages $Packages --port $Port --bind $Bind
if ($LASTEXITCODE -ne 0) { throw "QML site exited: $LASTEXITCODE" }
