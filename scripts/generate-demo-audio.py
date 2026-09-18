"""Generate three original, deterministic PCM compositions (CC0-1.0).
No recordings, samples or third-party melodies are used.
"""
import array
import json
import math
from pathlib import Path
import wave

target = Path(__file__).resolve().parents[1] / 'packages/elisa/assets/audio'
target.mkdir(parents=True, exist_ok=True)
catalog = []
for index, (identifier, title, notes) in enumerate([
    ('morning', '晨光序曲', [60, 64, 67, 71, 69, 67, 62, 64]),
    ('rain', '雨后漫步', [57, 60, 64, 67, 65, 64, 60, 59]),
    ('night', '城市夜航', [55, 62, 65, 69, 67, 65, 62, 59]),
]):
    rate, seconds = 22050, 24
    samples = array.array('h')
    for n in range(rate * seconds):
        t = n / rate
        beat = int(t / .75)
        local = t % .75
        frequency = 440 * 2 ** ((notes[beat % len(notes)] - 69) / 12)
        envelope = min(local / .015, 1) * math.exp(-local * 3.7)
        fade = min(t / .15, 1, (seconds - t) / .4)
        value = envelope * (math.sin(2 * math.pi * frequency * t)
                           + .18 * math.sin(4 * math.pi * frequency * t))
        value += .12 * math.sin(2 * math.pi * frequency / 2 * t)
        samples.append(int(10500 * value * fade))
    with wave.open(str(target / (identifier + '.wav')), 'wb') as output:
        output.setparams((1, 2, rate, 0, 'NONE', 'not compressed'))
        output.writeframes(samples.tobytes())
    catalog.append({'id': identifier, 'title': title, 'artist': 'Q-Browser 原创',
                    'album': ['清晨来信', '雨的记忆', '夜色电台'][index]})
(target / 'catalog.json').write_text(json.dumps(catalog, ensure_ascii=False, indent=2) + '\n', encoding='utf-8')
