"""Compare 424 exact allocation outputs from canonical and typed loaded-world replay.

Case IDs refer to the fingerprinted input grid and generated C++ header. Only
allocation-result events are selected. Rename op to tag on both sides; no ignored
fields, pointer normalization, tolerances or order relaxation. Service doubles
in the reference and deferred effects in the typed replay remain separate scope.
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
from allocation_reference import cases, header


def read(path):
    return [json.loads(line) for line in path.read_text(encoding='utf-8-sig').splitlines() if line.strip()]


def main():
    ap=argparse.ArgumentParser(description=__doc__);ap.add_argument('--reference',required=True,type=Path);ap.add_argument('--run',required=True,type=Path);args=ap.parse_args()
    reference=args.reference.resolve();run=args.run.resolve()
    report=json.loads((reference/'report.json').read_text())
    if report['result']!='PASS':raise SystemExit('Reference is unproven')
    for name,expected in report['fingerprints'].items():
        path=Path(name)
        if not path.is_absolute():path=REPO/path
        if hashlib.sha256(path.read_bytes()).hexdigest()!=expected:raise SystemExit('Reference input changed: '+name)
    inputs=json.loads((reference/'inputs.json').read_text());captured=json.loads((reference/'capture.json').read_text())
    original=read(reference/'original.jsonl')
    if inputs!=cases() or original!=captured['events'] or len(original)!=424:raise SystemExit('Reference events/inputs changed')
    if header(inputs,original)!=(REPO/'src/AllocationGolden.h').read_text():raise SystemExit('Generated replay inputs changed')
    ui=run/'profile-runtime/ui.jsonl';events=read(ui)
    if not events or events[-1].get('verdict')!='PASS' or events[-1].get('steps')!=92 or events[-1].get('failed')!=0:
        raise SystemExit('Requires successful complete 92-step profile')
    if 'RESULT: CLEAN' not in (run/'smoke.out').read_text():raise SystemExit('Missing clean runtime observation')
    rebuilt=[e for e in events if e.get('op')=='allocation-result']
    for rows in [original,rebuilt]:
        if len(rows)!=len(inputs) or any(e.get('case')!=i or e.get('kind')!=c['kind'] for i,(e,c) in enumerate(zip(rows,inputs))):
            raise SystemExit('Incomplete or reordered capture')
    out=run/'allocation-pair';out.mkdir(exist_ok=False)
    for label,rows in [('original',original),('reconstructed',rebuilt)]:
        normalized=[dict(tag=row['op'],**{k:v for k,v in row.items() if k!='op'}) for row in rows]
        (out/f'{label}.jsonl').write_text(''.join(json.dumps(row)+'\n' for row in normalized),encoding='utf-8')
    paths=[Path(__file__),REPO/'scripts/frida_trace_diff.py',reference/'report.json',reference/'capture.json',reference/'inputs.json',reference/'original.jsonl',
           ui,run/'run.json',run/'profile-runtime/transaction.json',REPO/'build/Debug/iwd2-re.exe',REPO/'build/Debug/iwd2-re.pdb',
           REPO/'src/CScreenCharacter.cpp',REPO/'src/CScreenCharacter.h',REPO/'src/CGameSprite.cpp',REPO/'src/CGameSprite.h',
           REPO/'src/AllocationChecks.h',REPO/'src/AllocationGolden.h']
    hashes={str(p):hashlib.sha256(p.read_bytes()).hexdigest() for p in paths}
    command=[sys.executable,str(REPO/'scripts/frida_trace_diff.py'),'compare',str(out/'original.jsonl'),str(out/'reconstructed.jsonl'),'--raw-pointers','--json']
    result=subprocess.run(command,cwd=REPO,capture_output=True,text=True,check=False)
    (out/'comparison.json').write_text(result.stdout,encoding='utf-8')
    same=hashes=={str(p):hashlib.sha256(p.read_bytes()).hexdigest() for p in paths}
    metadata=dict(result='MATCH' if result.returncode==0 and same else 'UNPROVEN',command=command,events=len(inputs),fingerprints=hashes,
                  inputs_unchanged=same,normalization='op -> tag only',scope=__doc__)
    (out/'metadata.json').write_text(json.dumps(metadata,indent=2),encoding='utf-8')
    print(result.stdout.strip());print(out);return 0 if result.returncode==0 and same else 2


if __name__=='__main__':raise SystemExit(main())
