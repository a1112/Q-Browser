# QML application demos

Run `scripts/start-qml-demos.ps1` on a Windows Qt 6.11 installation. Override
`-QtRoot`, `-CMake`, `-OpenSslRoot` as needed. It prepares an isolated protected
development deployment, signs three packages independently, installs each
through the production Host, then opens the example gallery. `-SkipBuild`
reuses an existing deployment. This development artifact has no production
release acceptance attestation; normal release builds retain their full checks.
Use `-Rebuild` after source changes, with the previous Demo Host closed.

| Route | Package | License |
| --- | --- | --- |
| app://pilot/demos/elisa | com.qbrowser.demo.elisa | LGPL-3.0-or-later; audio CC0 |
| app://pilot/demos/tokodon | com.qbrowser.demo.tokodon | GPL-3.0 |
| app://pilot/demos/coffee | com.qbrowser.demo.coffee | BSD-3-Clause |

Each package includes `PORTING.md`, pinned `UPSTREAM.json`, license texts and
modified QML source. The six Widgets examples remain available in trusted-shell
mode. QML cards explain when package execution is unavailable. Missing/corrupt
packages are rejected by the existing verified-launch flow.

## Runtime 1.3.0 audio capability

Optional manifest permission: `"audioPlayback": "package-assets"`. Absent means
denied; the effective policy requires both package and Host authorization.
`Runtime.invoke("audio", operation, payload)` completes through
`capabilityFinished(requestId, {ok, result})` or `{ok:false,error:{code,message}}`.

| Operation | Payload | Result |
| --- | --- | --- |
| catalog | {} | tracks with id, title, artist, album |
| play | {trackId} | actual playback status |
| pause / stop / status | {} | actual playback status |
| seek | {position: milliseconds} | actual playback status |
| setVolume | {volume: 0..1} | actual playback status |

Status contains trackId, state, position, duration, volume, ended, error.
Only exact catalog IDs from the current verified package are accepted. The Host
constructs `assets/audio/<id>.wav`; IPC cannot select a path, URL or other package.
Catalog size/count and WAV byte sizes are bounded. Files are pinned by stable
Windows handles. Qt Multimedia is in the Host closure only. No autoplay and no
simulated audio progress. Playback survives tab hiding/minimization, but its
authority is revoked when the owning runtime is retired. No output device and
backend errors are reported explicitly.

The reserved `/__demo_gallery` navigation is accepted only from an authenticated
Demo Worker and returns its owning tab to the trusted gallery. Ordinary Worker
routes retain same-package checks.

Tests: `tst_audio_broker`, manifest/policy tests, `tests/quick/tst_QmlDemos.qml`,
gallery, browser chrome and Host lifecycle suites. Real-device audio tests skip
only when the backend explicitly reports that no output device exists.
