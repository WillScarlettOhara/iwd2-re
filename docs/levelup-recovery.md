# Character level-up recovery

Targets: 5E56F0 LevelUp, 5E6A30 CheckDropSlot, 5D72D0 OnKeyDown.
Full bundles: tmp_levelup_context.md, tmp_drop_context.md,
tmp_characterkey_context.md (+ manifests); all target ASM read with sym.py.
OnKeyDown was absent from the Ghidra function table; create-functions created
it and refreshed 29,706 exports. The unrelated 443320 creation still fails.
Canonical MD5: 25cb3d8aa2ec648e94e328d8c467973e.

LevelUp applies feat/no-op skill bonuses, narrows remaining skill points to
BYTE, grants class/divine abilities, updates bard/sorcerer shared maxima and
counts by the unsigned difference, and leaves empty spell levels untouched.
Rage SPIN236 becomes SPIN260 with the old maximum/current/shared values.
Its button selects the highest eligible ability, tests quickSlotType==4,
narrows the remaining count to SHORT, and copies the low disabled-type BYTE
from active stats. Druid shapeshifts grow only for a positive delta. The first
Talos cleric level adds an unclamped BYTE five to electricity resistance.
Snapshots are deleted, equipment is rechecked, and an effect pass is requested.
Domain spells preserve the native class-flag bypass and alignment checks.
Paladin/monk order restrictions and the best-class animation group follow;
the animation comparison/store cover both WORDs, clearing field_22 on change.

CheckDropSlot keeps usable items, clears weapon/ammo-launcher shortcuts before
the removal attempt, then tries inventory and ground. Native ground fallback
queues a container-add message constructed from an empty CItem, not the moved
item; this oddity is retained. OnKeyDown handles popup Escape/Enter, portrait
keys, tooltips, portrait arrows, screenshot, eight panel bindings and rest;
Ctrl matches exactly, and the first matching binding terminates the search.

CScreenCharacter now packs to 2. Constructor ASM proves three omitted C2DArray
members and the BYTE at 1776; unknown gaps remain named byte storage.
field_2B6 aliases m_spells.m_nHighestLevel and no longer occupies extra bytes.
106 compiled member spacings PASS; vtable audit clean, 31/31 anchors.
Ctor never initializes field_1840; 5E3313 writes it later. This verified native
omission is the sole added arc-baseline entry, not an invented initialization.

Reference: scripts/probes/levelup_reference.py; tmp_levelup_native10/report.json
PASS: 408 LevelUp, 45 drop, 1,599 key cases. Probe SHA256
a14af1db491e29e4b52bdaabf36bf384977a63f0bc46c85642c191feaeedebc2.
Frozen original targets run unchanged with documented dependency doubles;
code/globals restore, private PID is killed, and fingerprints are rechecked.
This is orchestration evidence, not independent proof of doubled callees.
Typed LevelUpChecks uses real dependencies and borrowed pinned spell/rule data;
CharacterKeyChecks replays 309 safe key cases plus two-key dispatch via CWarp.
Six real item cases cover usable, cursed and rejected-to-inventory transitions.
Ground fallback and popup/screenshot keyboard cases have original-only evidence.

Full ARC 2026-10-18T14-03-54-164823-batch PASS with three call-count warnings:
134->86 /35->30 /22->40. Lints398s, build175s, 88/88 steps, 60s CLEAN;
hits408/9390/310, 4278 level-up/item checks +310 keyboard checks. Melee6HP,
arrow/bolt/stone12HP and ammo3->2; ordinary projectile arrivals3HP/source0.
Cleanup restored=true/errors[]; no running game. Generic static/differential
remain unproven; manual native-fixture scope is stated above. First run
2026-10-17T20-10-37-454604-batch stopped on a cleric-without-domain fixture;
inputs were corrected and recaptured, without weakening production behavior.

Next: OnLevelUpButtonClick 5E4E80 / caller 5E1910 to expose the recovered
completion through the normal level-up UI. Keep the complete combat profile.
