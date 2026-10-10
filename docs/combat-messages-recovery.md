# Combat message packets and reception

2026-10-10. Native Win32; disposable save `000000037-testSpells`, centered on the party.

## Recovered scope

- AddEffect Marshal `4F8470`, Unmarshal `4F8880`.
- FireProjectile Marshal `5004D0`, Unmarshal `5006C0`.
- Damage UsesDice `49FC40`: restore TRUE at damage vtable `84AE3C+1C`. This COMDAT return is shared with unrelated methods; the virtual golden-packet assertion proves the damage override runs.
- FireProjectile Run `5007F0`: restore Whirlwind `m_wanderSeed` from message `field_20` for zero-based type `0x130`.
- AsynchronousUpdate `4EE020` remains explicitly PARTIAL: add incoming factories 1 and 26 to existing inventory factories 2/34, preserving immediate stack ownership and deferred heap ownership.
- Both packet classes explicitly use pack(2); FireProjectile compiled member spacing passes. AddEffect has only one member, so the spacing auditor cannot compare it.

Full bundles: `tmp_combat_messages_named_ctx.md`, `tmp_fire_marshal_ctx.md`, `tmp_fire_run_ctx.md`, `tmp_damage_uses_dice_ctx.md`. Missing Marshal export created from its vtable. Full packet/Run assembly and factories `4EF62F`/`4EF3BC` checked. `4699F0` = existing spell SetResRef; `51DE20` = CPoint constructor.

## Wire behavior

Marshal writes into the caller's buffer, without the three-byte network header. Unmarshal starts after that header.

AddEffect body: target remote pair (8), ITEM_EFFECT fields through resource (28 total including target), source spell resref (8), dice or maximum/minimum levels, save mask/modifier, damage special, source point, source remote pair, target point, final BYTE = 89 bytes. Resource fields are zero-padded to eight bytes. Opcode/duration type narrow to WORD; target/spell level and probabilities narrow to BYTE. Duration flags are not merged from m_flags. UsesDice selects dice or level bounds. Missing marshal target yields zero size without writing. Missing decode target leaves state; missing non-sentinel source fails after retargeting. `(0,-1)` source is accepted. Decode reconstructs concrete effects, damage special, signed SHORT caster level, doubled feat 64 in m_firstCall, source resource and field_188. Message caller is retained.

FireProjectile body: caster remote pair, raw WORD factory type, projectile target remote pair, point, LONG height, full-width field_20, raw BYTE delay = 35 bytes. Missing marshal caster yields zero size; missing target emits `(0,-1)`. Decode accepts an unmapped caster as -1, but rejects an unmapped projectile target except `(0,-1)`, after updating caster/type. No extra type increment during decode. Run keeps ownership/area gates and launches through the concrete projectile virtual Fire.

No new malformed-length policy: original asserts consumed size after successful decoding. Matching invalid packets remain outside this recovery's test scope.

## Validation

Profile `scripts/profiles/combat-messages.json`, scenario `scripts/scenarios/combat-messages.txt`: virtual golden packets, field boundaries, caster reconstruction, partial failures; MG immediate/deferred reception; concrete launches removed before ordinary ticks. Received damage requires defender -3 HP, source unchanged. Melee, ranged ammunition and opcode-430 regressions remain required.

Initial RTC failure in CNetwork::CheckSessionStatus came from simulated sessions waking the transport thread. Receive fixtures keep local player ID zero until restoring the connection. Rerun `2026-10-10T20-36-36-435333-batch`: BUILD OK, 44 UI steps PASS, 56 packet checks, received damage -3 HP, melee -6 and each ranged Swing -12; CLEAN after 60s hold, 6/6 PDB gates. Cleanup restored all paths. Six static YELLOW call-count findings; layout checks skipped (manual/compiled audits above). No differential claim; dispatcher remains PARTIAL.

Next: grouped effects message 105 serialization/reception, then remaining combat message factories. The global incoming dispatcher is not claimed fully recovered.
