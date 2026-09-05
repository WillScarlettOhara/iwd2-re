"""Read-only calls to FindAvailableSpell on live sprites in our native build.

Run with the repository Python; accepts native.py smoke arguments (e.g.
--new-game --party 1 --hold 20). Reuses its spawn, crash oracle and cleanup.
This is injected method coverage, NOT proof that the casting UI calls it.
Only PDB symbols and reference arguments are used: no original-binary offsets
are applied to debug-build sprite state. Results: tmp_available_spell.jsonl.
"""
from pathlib import Path
import json
import sys

sys.path.insert(0, str(Path(__file__).resolve().parents[1]))
import frida_crash_guard
import native

PROBE = r"""
var findAddresses = resolve('CGameSprite::FindAvailableSpell');
var tickAddresses = resolve('CGameSprite::ProcessAI');
if (findAddresses.length !== 1 || tickAddresses.length !== 1)
    throw new Error('Expected one FindAvailableSpell and one sprite ProcessAI');
var findSpell = new NativeFunction(findAddresses[0], 'int',
    ['pointer','pointer','pointer','pointer','pointer'], {abi:'thiscall', traps:'all'});
var visited = {};
var visitedCount = 0;
function query(sprite, spell, cls, spec, level) {
    var res = Memory.alloc(8);
    res.writeByteArray([0,0,0,0,0,0,0,0]);
    for (var i=0; i<spell.length; i++) res.add(i).writeU8(spell.charCodeAt(i));
    var c=Memory.alloc(1), s=Memory.alloc(4), l=Memory.alloc(1);
    c.writeU8(cls); s.writeU32(spec); l.writeU8(level);
    var ok = findSpell(sprite, res, c, s, l);
    var row = {tag:'SPELL_PROBE', spell:spell, input:[cls,spec,level],
        found:ok, output:[c.readU8(),s.readU32(),l.readU8()]};
    if (ok !== 0 && ok !== 1) throw new Error('Invalid BOOL');
    if (!ok && JSON.stringify(row.input) !== JSON.stringify(row.output))
        throw new Error('Failed search modified output arguments');
    if (ok && (row.output[0] < 1 || row.output[0] > 11 ||
               row.output[2] < 1 || row.output[2] > 9))
        throw new Error('Successful search returned invalid class/level');
    send(row);
    return row;
}
Interceptor.attach(tickAddresses[0], { onEnter:function() {
    var sprite=this.context.ecx, key=sprite.toString();
    if (visited[key] || visitedCount >= 32) return;
    visited[key]=true; visitedCount++;
    query(sprite,'ZZNOSPL',255,0x12345678,255);
    ['SPPR','SPWI'].forEach(function(prefix) {
        for (var id=101; id<=130; id++) {
            var name=prefix+id;
            var row=query(sprite,name,0,0,0);
            if (row.found) {
                var hinted=query(sprite,name,row.output[0],row.output[1],row.output[2]);
                if (!hinted.found || JSON.stringify(hinted.output)!==JSON.stringify(row.output))
                    throw new Error('Explicit class/level did not reproduce available spell');
                query(sprite,name,0,1,0); // domain-only search
            }
        }
    });
}});
"""


def main():
    rows = []
    base = native.Observation
    with (native.REPO / 'tmp_available_spell.jsonl').open('w', encoding='utf-8') as output:
        class ProbeObservation(base):
            def message(self, message, data=None):
                payload = message.get('payload', {})
                if message.get('type') == 'send' and payload.get('tag') == 'SPELL_PROBE':
                    rows.append(payload)
                    output.write(json.dumps(payload) + '\n')
                    output.flush()
                else:
                    super().message(message, data)

        native.Observation = ProbeObservation
        frida_crash_guard.COUNT_JS += PROBE
        # Frida suppresses recursive onEnter delivery within this script's AI
        # callback, even with traps='all'. Count the driving callback normally;
        # direct lookup completion/return values are gated separately below.
        rc = native.main(['smoke', *sys.argv[1:], '--hit', 'CGameSprite::ProcessAI'])
    found = sum(r['found'] == 1 for r in rows)
    missed = sum(r['found'] == 0 for r in rows)
    print(f'PROBE: {found} successful searches, {missed} unsuccessful searches')
    return rc if rc else (0 if found and missed else 2)


if __name__ == '__main__':
    raise SystemExit(main())
