# Tokodon 26.08.1 → Q-Browser

Upstream: https://github.com/KDE/tokodon/tree/v26.08.1
Commit: `dd5073bf631c826345753e03f05f27945fd6925a`.

The real `src/qml/PostDelegate/InteractionButton.qml` is preserved and adapted:
Kirigami icon rendering becomes a text glyph, theme colors and spacing become
local values; tooltip, accessibility, interaction state, content row and count
remain. Original copyright and GPL-3.0-or-later notices are retained.
`UPSTREAM.json` records the upstream path and pre-modification SHA-256.

`Main.qml` is a new reduced GPL-3.0-only application shell using that component,
with a session-local ListModel replacing Tokodon's account/network models.
This is not a port of the entire original Main.qml. It supports feed, search,
details, favorites, bookmarks, replies and composing local posts. It makes no
network requests. Sample authors, avatars and posts are fictional original
demo data. Avatars and icons are QML shapes/text; no third-party image or font
assets are shipped. No KDE libraries are loaded.

Complete modified QML source is included in this package, with GPL texts in
`LICENSES`. Rebuild/repackage using `scripts/create-dev-package.ps1 -PackageName
tokodon` and your development trust key. No binary-only modified component is
distributed.
