"""Compare CSV reports from bench_audio; machine/scheduler noise is not filtered."""
import csv
import sys
from pathlib import Path

def read(path):
    with Path(path).open(encoding='utf-8-sig', newline='') as stream:
        return {(r['scenario'], r['sample_rate'], r['block'], r['chord_notes']): r for r in csv.DictReader(stream)}

if len(sys.argv) != 3:
    raise SystemExit('Usage: python tools/compare_audio_bench.py before.csv after.csv')
before, after = map(read, sys.argv[1:])
if before.keys() != after.keys():
    raise SystemExit('Reports do not contain the same workloads.')
out = csv.writer(sys.stdout)
out.writerow(['scenario','rate','block','chord_notes','active_voices_peak','before_p99_us','after_p99_us','p99_change_percent','before_note_max_us','after_note_max_us','before_overruns','after_overruns'])
for key, a in before.items():
    b = after[key]
    if a['active_voices_peak'] != b['active_voices_peak']:
        raise SystemExit('Active voice counts changed: ' + str(key))
    delta = 100 * (float(b['p99_us']) / float(a['p99_us']) - 1)
    out.writerow([*key,b['active_voices_peak'],a['p99_us'],b['p99_us'],f'{delta:.1f}',a['note_on_max_us'],b['note_on_max_us'],a['overruns'],b['overruns']])
