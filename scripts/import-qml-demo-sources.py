"""Reproduce the initial, pinned upstream extraction. Run before local adaptations only.

Requires sparse clones at build/upstream-{elisa,tokodon,qtdoc}. Does not fetch or
overwrite local adaptations unless explicitly run. Original assets are inventoried.
"""
from pathlib import Path
import hashlib
import json
import re
import shutil
import subprocess

ROOT = Path(__file__).resolve().parents[1]
PINS = {
    'elisa': ('536761f05b603a1763a2c613ae3b5d5f9ddde696', 'v26.08.1', 'KDE/elisa'),
    'tokodon': ('dd5073bf631c826345753e03f05f27945fd6925a', 'v26.08.1', 'KDE/tokodon'),
    'coffee': ('7c98216e33fa52c8137a439a2b2a4201640c3b2e', 'v6.11.0', 'qt/qtdoc'),
}

for name, (commit, tag, repo) in PINS.items():
    source = ROOT / 'build' / ('upstream-qtdoc' if name == 'coffee' else 'upstream-' + name)
    assert subprocess.check_output(['git', '-C', str(source), 'rev-parse', 'HEAD'], text=True).strip() == commit
    dest = ROOT / 'packages' / name
    (dest / 'qml').mkdir(parents=True, exist_ok=True)
    (dest / 'LICENSES').mkdir(exist_ok=True)
    licenses = ['BSD-3-Clause'] if name == 'coffee' else ['GPL-3.0-only', 'GPL-3.0-or-later'] if name == 'tokodon' else ['LGPL-3.0-or-later', 'GPL-3.0-only']
    for license in licenses:
        path = source / 'LICENSES' / (license + '.txt')
        if not path.exists() and license == 'GPL-3.0-only':
            path = ROOT / 'build/upstream-tokodon/LICENSES/GPL-3.0-only.txt'
        shutil.copyfile(path, dest / 'LICENSES' / (license + '.txt'))
    inventory = []
    def copy(path, target):
        data = path.read_bytes()
        inventory.append({'upstream': str(path.relative_to(source)).replace('\\', '/'),
                          'file': str(target.relative_to(dest)).replace('\\', '/'),
                          'sha256': hashlib.sha256(data).hexdigest()})
        target.parent.mkdir(parents=True, exist_ok=True)
        target.write_bytes(data)
    if name == 'coffee':
        origin = source / 'examples/demos/coffee'
        for path in origin.glob('*.qml'):
            copy(path, dest / 'qml' / path.name)
        for path in (origin / 'images').rglob('*'):
            if path.is_file(): copy(path, dest / 'qml/images' / path.relative_to(origin / 'images'))
        copy(source / 'REUSE.toml', dest / 'UPSTREAM-REUSE.toml')
        for path in (dest / 'qml').glob('*.qml'):
            text = path.read_text(encoding='utf-8').replace('QtQuick.Controls.Basic', 'QtQuick.Controls').replace('import QtQml', 'import QtQuick')
            text = text.replace('import QtQuick.Effects\n', '').replace('import Qt.labs.synchronizer\n', '')
            while re.search(r'\bMultiEffect\s*\{', text):
                m = re.search(r'\bMultiEffect\s*\{', text)
                end, depth = m.end(), 1
                while depth:
                    depth += (text[end] == '{') - (text[end] == '}')
                    end += 1
                text = text[:m.start()] + text[end:]
            text = text.replace('"Titillium Web"', '"Microsoft YaHei UI"')
            path.write_text(text, encoding='utf-8')
        (dest / 'qml/qmldir').write_text('singleton Colors 1.0 Colors.qml\nsingleton Config 1.0 Config.qml\n', encoding='utf-8')
    elif name == 'elisa':
        copy(source / 'src/qml/TrackBrowserDelegate.qml', dest / 'qml/TrackBrowserDelegate.qml')
    else:
        copy(source / 'src/qml/PostDelegate/InteractionButton.qml', dest / 'qml/InteractionButton.qml')
    manifest = {'schemaVersion': 1, 'appId': 'com.qbrowser.demo.' + name, 'version': '1.0.0',
                'entryPoint': 'qml/Main.qml', 'runtime': {'minVersion': '1.3.0', 'maxVersion': '1.x'},
                'imports': ['QtQuick', 'QtQuick.Controls', 'QtQuick.Layouts'],
                'permissions': {'process': False},
                'limits': {'packageBytes': 20971520, 'memoryMiB': 384, 'processes': 1},
                'routes': ['/demos/' + name]}
    if name == 'elisa': manifest['permissions']['audioPlayback'] = 'package-assets'
    (dest / 'manifest.json').write_text(json.dumps(manifest, indent=2) + '\n', encoding='utf-8')
    (dest / 'UPSTREAM.json').write_text(json.dumps({'repository': 'https://github.com/' + repo,
        'tag': tag, 'commit': commit, 'files': inventory}, indent=2) + '\n', encoding='utf-8')
