"""Validate shipped factory assets and their build/lookup wiring (stdlib only)."""
from pathlib import Path
import array
import hashlib
import json
import re
import sys
import wave

root = Path(__file__).resolve().parents[1]
source = (root / 'src/FactorySources.cpp').read_text()
cmake = (root / 'CMakeLists.txt').read_text()
items = re.findall(r'\{"([^"]+\.wav)","([^"]+)",(\d+)', source)
assert len(items) == 36 and len({name for name, _, _ in items}) == 36
seen = set()
for name, resource, midi in items:
    assert resource == name.replace('.', '_') and 0 <= int(midi) <= 127
    assert cmake.count('Excitations/' + name + '\n') == 1
    path = root / 'Excitations' / name
    with wave.open(str(path)) as w:
        assert (w.getnchannels(), w.getsampwidth(), w.getframerate()) == (1, 2, 48000)
        assert 2 <= w.getnframes() <= 480000
        data = w.readframes(w.getnframes())
    samples = array.array('h', data)
    if sys.byteorder != 'little':
        samples.byteswap()
    assert 15000 <= max(abs(x) for x in samples) <= 16500
    digest = hashlib.sha256(data).digest()
    assert digest not in seen
    seen.add(digest)
rows = json.loads((root / 'Excitations/factory_extension.json').read_text())
assert len(rows) == 24
assert [r['file'] for r in rows] == [name for name, _, _ in items[12:]]
for r in rows:
    with wave.open(str(root / 'Excitations' / r['file'])) as w:
        data = array.array('h', w.readframes(w.getnframes()))
        assert data[0] == 0 and data[-1] == 0
print('PASS: 36 distinct factory WAVs, PCM metadata, peaks, lookup/build wiring; 24 additions with faded endpoints.')
