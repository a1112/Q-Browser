# Elisa 26.08.1 → Q-Browser

Upstream: https://github.com/KDE/elisa/tree/v26.08.1
Commit: `536761f05b603a1763a2c613ae3b5d5f9ddde696`.

`UPSTREAM.json` inventories the original file and SHA-256. The actual
`src/qml/TrackBrowserDelegate.qml` is adapted in `qml/TrackBrowserDelegate.qml`:
retain track properties, playback/queue signals, accessibility and keyboard
enqueue; replace Kirigami actions, metadata services and platform icons with
Qt Quick Controls and local signals. `Main.qml` is a new reduced application
shell, not a verbatim port of Elisa's complete window. Albums, search and queue
use a local catalog. No KDE libraries are loaded.

QML is LGPL-3.0-or-later; original copyright notices are retained. The modified
QML is distributed as editable source in this package. License texts are under
`LICENSES`. Recipients can modify/repackage the QML and sign with their own
development trust key using the repository's `create-dev-package.ps1`.

All three WAV tracks and the catalog are original Q-Browser compositions,
dedicated under CC0-1.0. They are synthesized deterministically by
`scripts/generate-demo-audio.py`; no sampled recordings or third-party melodies
are used. Album covers are new QML shapes and font glyphs, not upstream images.
No external fonts, artwork or icons are included.

Audio uses Runtime 1.3.0 and the optional `audioPlayback: package-assets`
permission. The Host owns playback; Worker imports remain unchanged.
