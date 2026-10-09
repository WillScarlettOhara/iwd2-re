# Script target and health predicates

Recovery lot following [sprite target resolution](combat-target-resolution-recovery.md).
Next lot: [object and door approach movement](combat-target-movement-recovery.md).
`CGameAIBase::EvaluateStatusTrigger` (`0x453840`) remains a partial dispatcher;
this lot restores 23 additional cases. `QuickDecode` (`0x45D050`) is an unchanged,
fully recovered dependency included in the runtime symbol gate.

## Recovered behavior

* `SetSpellTarget` (`0x40F4`, branch `0x45A8E9`) and `SetLastMarkedObject`
  (`0x40E1`, branch `0x45A8A0`) copy the resolved object's complete AI type to
  `field_3BA` and `field_342`, respectively. They use untyped `GetObject(FALSE)`
  after decoding a private trigger copy. A missing target stores `NOONE`;
  both branches return TRUE and release a successful share. Existing `Set`
  semantics copy active geometry and normalize the class mask; inactive geometry
  is not part of the copy contract. They do not call
  the action-target resolver or update combat reticles.
* Alignment, General, Class, Race, Specifics and Gender use the decoded selector.
  Attribute bytes compare against the full LONG trigger value. Class alone
  narrows the argument to BYTE before `IsClassValid`. Alignment values
  1/2/3/0x10/0x20/0x30 use a nonzero bit intersection, including the original
  mixed-axis behavior; other values compare for equality.
* HP, HPGT/HPLT, HPPercent and its GT/LT variants, and HPLost and its GT/LT
  variants use virtual `QuickDecode`. Current and maximum HP are signed SHORTs;
  arithmetic and comparison are signed LONGs. Percent uses `current * 100 /
  maximum`, truncating toward zero. HPLost is maximum minus current without
  clamping, so overhealing yields a negative result. No invented zero-denominator
  guard changes the original division behavior.
* StateCheck/NotStateCheck use the derived general-state mask;
  IsCreatureAreaFlag uses the base creature flags. CheckStat and its GT/LT
  variants use `GetAtOffset` with the stat ID narrowed to SHORT. All missing targets return FALSE,
  including NotStateCheck. Successful QuickDecode results release their share
  only when its return value is FALSE, as in the common PE epilogue.

## Static provenance

Read the entire 6,312-line decompile, required call set, BG2 layout and existing
header in `tmp_combat_set_spell_target_ctx.md` before implementation. Its SHA256
is `4540d3977f93168564b9f7692a1bd1f6b63b6a4c6d5f27fae99a51f172461c01`;
the export `.ghidra-exports/00453840.json` is
`33a690bad0135b9b53ccff21111cb0993050e11009d95a24a67fce409c7368cc`.
The header fingerprint is
`505efe85471fca3a512a70a09acd3f35526576a71a7b57d6915d6c01f1ec58ff`.
Also read the complete separate QuickDecode bundle and verified the unchanged
source dependency. The canonical PE SHA256 is
`18e01d21855f88702c062497aa25dfb42928671973d3b929e42206def2d9df2a`.

Assembly from the valid dispatcher entry supplies branch boundaries, signed
MOVSX/IDIV and comparison instructions, BYTE class narrowing, both target-field
setters and the conditional ReleaseShare epilogue. Post-return switch data is
excluded from instruction evidence. The earlier exploratory disassembly starting
at `0x453BE0` landed inside an instruction; it was discarded and replaced by
entry-based decoding (`0x453BDE` is the actual Alignment branch start).

## Validation

`scripts/profiles/combat-target-triggers.json` extends the previous profile with
`targettriggerchecks`. It checks stored selector 51/55 round trips, stale/NOONE/
Myself inputs, input immutability, selector width and mask quirks, signed HP
extremes and literal expected percentages, equality/strict inequalities, derived
state versus base flags, stat thresholds, missing targets and the NOT_SPRITE
sentinel. Actor state is restored within the same tick; message/reticle and deny
counts are checked for side effects.

Actual Swing tests now also require HP, HPLT, HPLost and HPPercent to agree with
the observed defender HP after damage delivery. The scenario retains all previous
target, formation, marked-spell, projectile and combat regressions, recenters the
loaded save and never saves the disposable process. The profile transaction owns
save/config/fixture backup and restoration.

Final batch `.arc/2026-10-09T22-23-48-664445-batch` passed all 37 scenario steps
and 2,427 assertions: 863 new target/health checks, 20 checks against actual
Swing damage, and 1,544 retained regression checks. Melee reduced defender HP
from 291 to 285, SwingOnHit to 278, bow to 266, crossbow to 254, and sling to
242. All five damage deliveries passed the four health predicates. The retained
opcode-430 instant/arrow/bolt/sling paths and ammunition consumption also passed.

The Win32 Debug build passed without warnings. The native 60-second crash watch
was CLEAN; both required symbols were observed (`EvaluateStatusTrigger`: 1,948
hits, unchanged `QuickDecode`: 407 hits). All eight source checks and class
audits passed. The profile transaction reports `restored: true` and no cleanup
errors; fixture cleanup and preserved-file hashes passed.

Static parity remains YELLOW (dispatcher 184 binary calls / 122 source calls;
QuickDecode 4 / 8), with an inherited argument-provenance count mismatch in the
partial dispatcher. Offset parity was skipped as not analysable (no header
offsets or no thiscalls), and layout analysis had no analysable layout. These static gates
remain unproven. A clean recovered run does not establish a differential
comparison with the original executable.

## Continuation

Recover ordinary MoveToObject action 22 and the door approach helper `0x48B2C0`.
Range currently retains an explicit deferred door branch. The PE door callers
pass a stack point whose initialization still needs to be established; do not
substitute the caller position or a door center without evidence.
