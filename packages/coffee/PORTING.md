# Qt Coffee Machine 6.11.0 → Q-Browser

Upstream: https://github.com/qt/qtdoc/tree/v6.11.0/examples/demos/coffee
Commit: `7c98216e33fa52c8137a439a2b2a4201640c3b2e`.

The original ApplicationFlow, all associated QML/forms and cup/ingredient/icon
assets were extracted. `UPSTREAM.json` lists every imported file and its
original SHA-256. `UPSTREAM-REUSE.toml` retains the upstream license mapping:
the `examples/**` annotation licenses these assets as
LicenseRef-Qt-Commercial OR BSD-3-Clause. This distribution chooses BSD-3-Clause;
the full text is in `LICENSES/BSD-3-Clause.txt`. Copyright remains The Qt Company.
The upstream Qt logo is retained as source attribution, not Q-Browser branding.

Modifications: ApplicationWindow becomes an embedded Rectangle; add the Host
gallery return; translate user-facing labels to Chinese; replace Basic imports
with the allowed QtQuick.Controls import; replace QtQml imports with QtQuick;
replace Qt.labs.synchronizer with Binding/Connections; remove MultiEffect shadow
blocks; use a local singleton qmldir; choose portrait/landscape by actual window
size; replace the unavailable external font with a system font; make completion
explicitly restartable.
Static dark-theme asset paths replace dynamic source expressions; the theme
toggle is hidden to preserve the Worker's existing static-URL policy.
Original drink selection, recipes, StackView flow and cup/progress animations
remain. No device or network connections are made.

All modified QML and imported assets are included as source. Fonts and native
code from the upstream example are not distributed. The original pre-extraction
version can be reproduced with `scripts/import-qml-demo-sources.py`; do not run
that initial extractor over local modifications you wish to preserve.
