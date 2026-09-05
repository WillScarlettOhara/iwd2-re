"""Gate successful spell lookup inside the normal ForceSpellPointAction call.

Uses PDB function symbols and reference arguments, no debug-build field offsets.
Run with native smoke args, normally --new-game --party 1 --hold 35
--ui scripts/scenarios/force-spell-point.txt. The scenario queues an engine
action; unlike the lookup-only probe this script never invokes a game function.
"""
import json
from pathlib import Path
import sys

sys.path.insert(0, str(Path(__file__).resolve().parents[1]))
import frida_crash_guard
import native

PROBE = r"""
var depth = {};
var projectile = {};
var handler = resolve('CGameAIBase::ForceSpellPointAction');
var lookup = resolve('CGameSprite::FindAvailableSpell');
var decode = resolve('CProjectile::DecodeProjectile');
if (handler.length !== 1 || lookup.length !== 1 || decode.length !== 1)
    throw new Error('Expected exactly one handler, lookup and projectile decoder');
Interceptor.attach(handler[0], {
    onEnter:function() {
        depth[this.threadId]=(depth[this.threadId]||0)+1;
        projectile[this.threadId]=false;
    },
    onLeave:function() { depth[this.threadId]--; }
});
Interceptor.attach(decode[0], {
    onEnter:function() { this.active=!!depth[this.threadId]; },
    onLeave:function(ret) {
        if (this.active) projectile[this.threadId]=!ret.isNull();
    }
});
Interceptor.attach(lookup[0], {
    onEnter:function(args) {
        this.active=!!depth[this.threadId];
        if (this.active) { this.cls=args[1]; this.spec=args[2]; this.level=args[3]; }
    },
    onLeave:function(ret) {
        if (this.active) send({tag:'FORCEPOINT_LOOKUP', found:ret.toInt32(),
            cls:this.cls.readU8(), spec:this.spec.readU32(), level:this.level.readU8(),
            projectile:!!projectile[this.threadId]});
    }
});
"""


def main():
    rows = []
    base = native.Observation
    with (native.REPO / 'tmp_forcepoint.jsonl').open('w', encoding='utf-8') as output:
        class ForcePointObservation(base):
            def message(self, message, data=None):
                payload = message.get('payload', {})
                if message.get('type') == 'send' and payload.get('tag') == 'FORCEPOINT_LOOKUP':
                    rows.append(payload)
                    output.write(json.dumps(payload) + '\n')
                    output.flush()
                else:
                    super().message(message, data)

        native.Observation = ForcePointObservation
        frida_crash_guard.COUNT_JS += PROBE
        rc = native.main(['smoke', *sys.argv[1:], '--hit',
                          'CGameAIBase::ForceSpellPointAction,CGameSprite::FindAvailableSpell'])
    successes = sum(r['found'] == 1 and 1 <= r['level'] <= 9 and r['projectile'] for r in rows)
    print(f'FORCEPOINT: {successes} successful in-handler lookups with projectile / {len(rows)}')
    return rc if rc else (0 if successes else 2)


if __name__ == '__main__':
    raise SystemExit(main())
