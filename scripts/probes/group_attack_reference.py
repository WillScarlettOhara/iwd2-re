"""Original GroupAttack semantic checks, compared with a recorded AutoUI run.

The comparison is of fourteen named predicates, plus fifteen with --target
and seven dispatch predicates with --dispatch (which includes --target),
not complete raw actor state.
The original is loaded using the existing driver; only this probe's PID is killed.
"""
from __future__ import annotations

import argparse
import hashlib
import json
import sys
import time
from datetime import datetime, timezone
from pathlib import Path

SCRIPTS = Path(__file__).resolve().parents[1]
sys.path.insert(0, str(SCRIPTS))
from arc_profile import ProfileSession, load_profile
from frida_orig import build_js
from runtime_paths import REPO, game_directory
import frida_trace_diff as diff


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--ours', type=Path, required=True, help='Existing combat-group or combat-target ui.jsonl')
    parser.add_argument('--timeout', type=int, default=150)
    parser.add_argument('--target', action='store_true', help='Also compare the fifteen UpdateTarget predicates')
    parser.add_argument('--dispatch', action='store_true', help='Also compare enemy dispatch; includes --target')
    args = parser.parse_args()
    args.target = args.target or args.dispatch
    if not 1 <= args.timeout <= 300:
        parser.error('timeout must be in 1..300 seconds')
    import frida
    game = game_directory()
    exe = game / 'IWD2.exe'
    canonical = '25cb3d8aa2ec648e94e328d8c467973e'
    if any(hashlib.md5(p.read_bytes()).hexdigest() != canonical for p in (exe, REPO / '.bin/IWD2.exe')):
        raise ValueError('Original executable is not canonical')
    ours = [dict(tag='combatgroup-case', case=e['case'], passed=e['passed'])
            for line in args.ours.read_text(encoding='utf-8').splitlines()
            if (e := json.loads(line)).get('op') == 'combatgroup-case'
            and (args.dispatch or not args.target or not e['case'].startswith('dispatch-'))]
    expected_count = 14 + (15 if args.target else 0) + (7 if args.dispatch else 0)
    if len(ours) != expected_count or not all(e['passed'] is True for e in ours):
        raise ValueError(f'Expected {expected_count} successful checks')
    out = REPO / '.arc' / datetime.now(timezone.utc).strftime('%Y-%m-%dT%H-%M-%S-%f-group-reference')
    out.mkdir()
    report = {'verdict':'UNPROVEN', 'scope':f'{expected_count} named predicates, not complete raw state',
              'ours_capture':str(args.ours.resolve()), 'frida_version':frida.__version__}
    paths = [exe, Path(__file__), Path(__file__).with_suffix('.js'), args.ours,
             SCRIPTS / 'frida_orig.py', SCRIPTS / 'arc_profile.py', SCRIPTS / 'frida_trace_diff.py']
    target_js = Path(__file__).with_name('update_target_reference.js')
    if args.target:
        paths.append(target_js)
    dispatch_js = Path(__file__).with_name('group_dispatch_reference.js')
    if args.dispatch:
        paths.append(dispatch_js)
    report['sha256'] = {str(p.resolve()):hashlib.sha256(p.read_bytes()).hexdigest() for p in paths}
    def write_report():
        (out / 'run.json').write_text(json.dumps(report, indent=2)+'\n', encoding='utf-8')
    write_report()
    events = []
    device = frida.get_local_device()
    pid = None
    session = None
    def stop():
        nonlocal pid
        if pid is not None:
            try:
                device.kill(pid)
            except frida.ProcessNotFoundError:
                pass
            pid = None
    profile = load_profile(REPO / 'scripts/profiles/combat-group.json', REPO)
    profile['preserve'] = ['save']
    driver = build_js({'hooks':[]}, 0, 10)
    call = 'LoadGame(this.thiz, LOAD_SLOT);'
    if driver.count(call) != 1:
        raise ValueError('Load driver changed; review exact-save integration')
    driver = driver.replace(call, 'gaSelectSave(this.thiz); ' + call)
    code = driver + '\n'
    if args.target:
        code += target_js.read_text(encoding='utf-8') + '\n'
    if args.dispatch:
        code += dispatch_js.read_text(encoding='utf-8') + '\n'
    code += Path(__file__).with_suffix('.js').read_text(encoding='utf-8')
    (out / 'original.js').write_text(code, encoding='utf-8')
    print(f'GROUP-REFERENCE {out}', flush=True)
    rc = 2
    try:
        with ProfileSession(profile, REPO, game, out, stop=stop) as transaction:
            transaction.start()
            with (out / 'original.raw.jsonl').open('w', encoding='utf-8') as log:
                def receive(message, data):
                    import os
                    event = message.get('payload') if message['type'] == 'send' else {'tag':'reference-error', 'message':message}
                    events.append(event)
                    log.write(json.dumps(event)+'\n'); log.flush(); os.fsync(log.fileno())
                try:
                    pid = device.spawn([str(exe)], cwd=str(game))
                    session = device.attach(pid)
                    script = session.create_script(code)
                    script.on('message', receive)
                    script.load()
                    device.resume(pid)
                    deadline = time.monotonic() + args.timeout
                    progress = time.monotonic() + 30
                    while time.monotonic() < deadline:
                        if any(e.get('tag') in ('reference-done','reference-error','crash')
                               or (e.get('tag') == 'drive' and e.get('step') == 'clickFailed')
                               for e in events):
                            break
                        if time.monotonic() >= progress:
                            print(f'Waiting for original: {len(events)} events', flush=True)
                            progress += 30
                        time.sleep(0.2)
                finally:
                    stop()
                    if session is not None:
                        try:
                            session.detach()
                        except frida.InvalidOperationError:
                            pass
        original = [e for e in events if e.get('tag') == 'combatgroup-case']
        errors = [e for e in events if e.get('tag') in ('reference-error','crash','ERROR')
                  or (e.get('tag') == 'drive' and e.get('step') == 'clickFailed')]
        if errors or not any(e.get('tag') == 'reference-done' for e in events) or len(original) != expected_count:
            raise ValueError(f'Incomplete original capture: checks={len(original)}, errors={errors}')
        for label, records in [('original', original), ('reconstructed', ours)]:
            (out / f'{label}.jsonl').write_text(''.join(json.dumps(e)+'\n' for e in records), encoding='utf-8')
        comparison = diff.compare(diff.load_trace(out / 'original.jsonl', diff.Options()),
                                  diff.load_trace(out / 'reconstructed.jsonl', diff.Options()))
        (out / 'differential.json').write_text(json.dumps(comparison, indent=2)+'\n', encoding='utf-8')
        report['verdict'] = comparison['verdict']
        report['checks'] = len(original)
        rc = 0 if report['verdict'] == 'MATCH' else 1
    except (Exception, KeyboardInterrupt) as error:
        report['error'] = f'{type(error).__name__}: {error}'
    finally:
        stop()
        write_report()
    print(json.dumps(report, indent=2), flush=True)
    return rc


if __name__ == '__main__':
    raise SystemExit(main())
