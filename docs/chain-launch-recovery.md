# Initial chain launch

Recovered DecodeProjectile 51EAF0 case D2, handler 5250B0. It allocates
native size 2BE, creates an empty signed-ID set with 537010, passes it to
57C510, destroys the temporary with 5370C0, then reaches the common tail
528E1C, which stamps WORD D1. No caster/delay-specific changes. Child hops
still call the constructor directly, retaining default type zero.

Correction: findptr 850F14 returned 70AAA5 because bytes across a mov/jne
boundary happen to form that DWORD. Disassembly proves no vtable assignment
there. UpdateSpriteEffect 709B60 is unrelated visual ticking, not the initial
chain launch. Removed that incorrect header/continuation claim.

Full bundle/manifests tmp_chain_launch_context.md (51EAF0/57C510); unchanged
5395-line factory decompile and shared CProjectile header/layout were read in
prior beam/chain recoveries. New handler, empty-set helpers and common tail
checked in native assembly; jump-table entry 5291C0 points to 5250B0. The
Ghidra caller map omits this constructor because the huge switch is split;
sym.py callsites finds the real E8 at 5250F9 and child call at 57C99D.

scripts/probes/chain_factory_reference.py executes canonical original factory
dispatch, common tail and actual empty-set construction/destruction. Six
variants: null/non-null caster, delay 0/17/255. Checks one constructor call,
empty self-linked head/nil parent, allocations 2BE/14/14, returned allocation,
type D1 and head/nil release order. Allocator, locks, CString destruction,
visual initialization and chain constructor are isolated callbacks; not a
resource/flight/damage oracle. Original stays suspended and is killed.
tmp_chain_factory_reference_final/report.json PASS6, canonical MD5,
globals/hooks restored, stable fingerprints, no cleanup or script errors.

AutoUI nearestchecks now starts through DecodeProjectile and virtual Fire,
not a hand-built carrier/explicit DeliverEffects. Private non-AI origin at
(48,96), real party targets (137,96)/(141,96)/(143,96), first flight beyond
40-unit speed. Virtual AIUpdate must actually move before arriving. Time Stop
must retain position/payload and queue no effect. Ordinary arrival invokes
chain delivery and deletes the parent; require DELETED object-array entry.
Each child must use parent ID as source, next target and default type zero.
Bounded tick loop consumes only fixture pending nodes and processes owned
group/add-effect messages. Real ZRCHIT payload requires six HP lost on each
of three defenders, none on caster, no persistent instant effects, no carrier
left. Original 49 native selection cases and all weapon/beam tests retained.

All actor/area/visibility/stats, Time Stop, queues and clock are restored in
one outer tick. This exercises production virtual flight/arrival code in a
controlled typed fixture; it does not prove player spell UI/resource wiring or
natural area scheduling. No save is written.

Validation .arc/2026-10-13T18-59-54-696898-batch PASS with four warnings:
Win32 Debug, all 70 UI steps, 81 nearest/launch checks including 49 original
selection cases, 58 chain and 88 beam checks. Initial flight arrives after
three ticks; child flights after one each. Three defenders lose six HP each,
caster unchanged. 60s CLEAN, factory/ctor/delivery/AI/Fire PDB hits
36/25/14/29/39. Melee/bow/crossbow/sling and packet/on-hit regressions pass.
Profile cleanup restored hashes. No gates reduced. Delivery parity GREEN;
other four YELLOW call-count mismatches (split factory/STL/inlined calls).
Missing offset oracles leave full static equivalence unproven; visual/player
spell presentation not claimed. Native reference verifies factory semantics,
not the complete gameplay path. Next: identify/recover impact visual dispatch
52E160 (native vtable reference 84D9C0), then verify its production callers.
