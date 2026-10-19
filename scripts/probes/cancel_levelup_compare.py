"""Compare the 321 canonical done gates with one complete AutoUI profile run.

Only the two named gate events are selected. Rename op to tag on both sides;
retain every other field, event order, exact counts and numeric values. No
pointer normalization, tolerance or ignored fields. Other target behavior is
outside this differential receipt.
"""
from __future__ import annotations
import argparse
import hashlib
import json
import subprocess
import sys
from pathlib import Path

sys.path.insert(0,str(Path(__file__).resolve().parents[1]))
from runtime_paths import REPO
from cancel_levelup_reference import gate_cases, header


def read(path):
    return [json.loads(line) for line in path.read_text(encoding='utf-8-sig').splitlines() if line.strip()]


def main():
    ap=argparse.ArgumentParser(description=__doc__.splitlines()[0])
    ap.add_argument('--reference',required=True,type=Path)
    ap.add_argument('--run',required=True,type=Path)
    args=ap.parse_args()
    reference=args.reference.resolve();run=args.run.resolve()
    report=json.loads((reference/'report.json').read_text())
    if report['result']!='PASS':raise SystemExit('Reference is not proven')
    for name,expected in report['fingerprints'].items():
        path=Path(name)
        if not path.is_absolute():path=REPO/path
        if hashlib.sha256(path.read_bytes()).hexdigest()!=expected:raise SystemExit('Reference input changed: '+name)
    captured=json.loads((reference/'capture.json').read_text())
    if header(captured)!=(REPO/'src/CancelLevelUpGolden.h').read_text():raise SystemExit('Golden inputs changed')
    ui=run/'profile-runtime/ui.jsonl';events=read(ui)
    if not events or events[-1].get('verdict')!='PASS' or events[-1].get('steps')!=90 or events[-1].get('failed')!=0:
        raise SystemExit('Requires complete successful 90-step profile')
    if 'RESULT: CLEAN' not in (run/'smoke.out').read_text():raise SystemExit('Missing clean runtime observation')
    tags={'native-done-gate','native-done-text'}
    original=read(reference/'done-gates.jsonl')
    expected=[dict(op='native-done-gate',case=i,input=row,result=value) for i,(row,value) in enumerate(zip(gate_cases(),captured['gates']))]
    expected += [dict(op='native-done-text',case=i,result=value) for i,value in enumerate(captured['texts'])]
    if original!=expected:raise SystemExit('Reference events changed')
    rebuilt=[e for e in events if e.get('op') in tags]
    for rows in [original,rebuilt]:
        if len(rows)!=321 or [row['op'] for row in rows]!=['native-done-gate']*308+['native-done-text']*13:
            raise SystemExit('Incomplete or reordered gate capture')
    out=run/'cancel-levelup-pair';out.mkdir(exist_ok=False)
    for name,rows in [('original',original),('reconstructed',rebuilt)]:
        normalized=[dict(tag=row['op'],**{k:v for k,v in row.items() if k!='op'}) for row in rows]
        (out/f'{name}.jsonl').write_text(''.join(json.dumps(row)+'\n' for row in normalized),encoding='utf-8')
    paths=[Path(__file__),REPO/'scripts/frida_trace_diff.py',reference/'report.json',reference/'capture.json',reference/'done-gates.jsonl',
           ui,run/'run.json',run/'profile-runtime/transaction.json',REPO/'build/Debug/iwd2-re.exe',REPO/'build/Debug/iwd2-re.pdb',
           REPO/'src/CScreenCharacter.cpp',REPO/'src/CScreenCharacter.h',REPO/'src/CancelLevelUpChecks.h',REPO/'src/CancelLevelUpGolden.h']
    hashes={str(path):hashlib.sha256(path.read_bytes()).hexdigest() for path in paths}
    command=[sys.executable,str(REPO/'scripts/frida_trace_diff.py'),'compare',str(out/'original.jsonl'),str(out/'reconstructed.jsonl'),'--raw-pointers','--json']
    result=subprocess.run(command,cwd=REPO,capture_output=True,text=True,check=False)
    (out/'comparison.json').write_text(result.stdout,encoding='utf-8')
    same=hashes=={str(path):hashlib.sha256(path.read_bytes()).hexdigest() for path in paths}
    metadata=dict(result='MATCH' if result.returncode==0 and same else 'UNPROVEN',command=command,events=321,
                  fingerprints=hashes,inputs_unchanged=same,selection=sorted(tags),normalization='op -> tag only',scope=__doc__)
    (out/'metadata.json').write_text(json.dumps(metadata,indent=2),encoding='utf-8')
    print(result.stdout.strip());print(out)
    return 0 if result.returncode==0 and same else 2


if __name__=='__main__':raise SystemExit(main())
