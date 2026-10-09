# Object and door approach movement

Recovery following [target and health predicates](combat-target-trigger-recovery.md).

## Recovered behavior

* `CGameDoor::GetMoveDest` (`0x48B2C0`) returns a reference to either existing
  approach point. It compares `dx * dx + dy * dy * 16 / 9` using signed LONG
  intermediates and truncation toward zero. Each multiplication/addition retains
  the PE's 32-bit wrapping. An equality chooses the second point. It changes
  neither the input point nor the door and makes no calls.
* Ordinary MoveToObject action 22 in the partial sprite ExecuteAction dispatcher
  (`0x728F80`, case entry `0x72B229`) requires an exact sprite source and uses
  `ResolveCurrentActionTarget`, not the base typed resolver. Missing, stale and
  immune targets finish DONE. Immunity still occurs after target publication.
  Sprite targets set source base flag `0x100000` when their allegiance is at or
  below GOODCUTOFF, invoke MoveToObject, and store last action ID 22 even on an
  executor error. Non-sprite targets use their position, doors their approach
  point, and invoke MoveToPointRange with zero range. Successful resolutions
  release their share; every branch stores the last action return.
* The Range trigger in partial EvaluateStatusTrigger (`0x453840`, branch
  `0x454851`) now uses the door helper instead of the previous plain-position
  approximation. Its existing comparisons and marked-object publication remain.
* MoveToPointRange (`0x73FEC0`) had two incorrect source branches. Nonzero range
  uses a boolean Y term: **one when the Y cells are equal, zero otherwise**,
  rather than a geometric delta. The existing source had this reversed.
  Without a path/request, BACKGROUND and action count at least one return ERROR.
  Eligible identical destinations return INTERRUPTABLE without scheduling,
  including normal actions with a positive count. Both source gates are corrected.
  The existing front/back request construction remains unchanged.

## Default-constructed door caller input

Both original callers pass a stack CPoint before writing its coordinates.
ExecuteAction's entry and case path contain no writes to `[esp+0x14/0x18]` before
the call at `0x72B2B2`. Range calls the folded default constructor at `0x68BFF0`
for `[esp+0xD8/0xDC]` before its call at `0x4548BC`; that constructor consists of
`mov eax, ecx; ret` and does not initialize anything. The address-map name
CGameArea::GetHeader describes a folded alias, not the semantics of this call.
The full entry-based paths were inspected, excluding jump tables after returns.

The recovered callers retain a default-constructed CPoint and do not invent a
source position, door center or zero initialization. As in the original,
selection between distinct points depends on unspecified stack contents.
Deterministic caller tests set the two door approach points equal. Direct helper
tests provide initialized inputs and verify which member reference is returned.
No claim of matching original/recovered stack residues is made.

## Static provenance

Full named door context, header and BG2 layout were assembled and read before
writing the body: `tmp_combat_door_named_ctx.md`, SHA256
`185933a0b1c6ebbdbc5d888eca27a88b5bfd247732814a956967e9a45f97089a`.
The door export fingerprint is
`9a77002c25f4d4c392f00c79998fdb4e2d70b7e6c9d8cf8d7442116c11b8ac4b`.
PE instructions `0x48B2C0..0x48B34A` establish signed comparisons, division,
wrapping and tie choice; the BG2 PDB confirms `const CPoint& GetMoveDest(const CPoint&)`.

Both complete movement executors and required call sets were read in
`tmp_combat_move_executors_ctx.md`, SHA256
`a19821b2a64b70903b1871bbd0e2d4d4603771dec027861fec43422e792d66b6`.
The sprite shared BG2 layout and complete header are reused from the preceding
lot: layout block `47cbd251`, header file SHA256
`ebb579e035b15c4ab5bc47923b9b05ab5177fc6f1d5fa1eb7591d9ac7facd4b6`.
ExecuteAction's full decompile was previously read; the matching export SHA256
`5dee122f055c7e832bf4ad6a124c0ce1fa479ba83ecd4fe2c73f7c46a697ebe7`
is explicitly reused, and its new complete required-call block was read in
`tmp_combat_move_dispatch_ctx.md`. EvaluateStatusTrigger's complete context and
export `33a690ba` are reused from the preceding lot. Entry-based disassembly
establishes the new dispatcher path, and MoveToPointRange's `NEG/SBB/INC` Y term
and background/destination gates. No virtual slots or object members change.

## Validation

The movement profile retains the complete combat scenario and extends
`targetresolvechecks` with actual target-ID and point searches, signed throttle
wrapping, action-22 flags/returns/immunity, literal point-range thresholds, area
edges, background eligibility, and initialized door distance/reference tests.
Front and back queues are inspected under the search critical section, with a
private semaphore preventing a wake of the production worker after requests are
removed. Actor state, door points, messages and queue baselines are restored.

Command:

```powershell
& .\.venv-reagent\Scripts\python.exe scripts/arc.py batch 0x48B2C0 0x73FEC0 0x728F80 0x453840 0x73EDD0 --profile scripts/profiles/combat-target-movement.json
```

Batch `.arc/2026-10-09T22-36-53-201705-batch` ran all 37 steps and passed all
2,650 assertions, including 223 new checks. Target/formation/movement checks now
total 414; the previous lot's 863 target/health checks and all actual melee,
bow, crossbow, sling, on-hit and opcode-430 regressions pass unchanged.
The native 60-second crash watch is CLEAN and all five PDB gates execute:
GetMoveDest 19 hits, MoveToPointRange 31, ExecuteAction 93,466,
EvaluateStatusTrigger 1,952 and unchanged MoveToObject 31. The complete Win32
Debug rebuild passes (161 seconds); the pre-existing IcewindCGameEffects DWORD
to USHORT warning remains. Eight source checks, the sprite/base class audits
and native sprite layout checks pass. The transaction reports restored=true,
cleanup_errors=[], with preserved-path hash verification and fixture cleanup.

**The overall batch verdict is FAIL**, due to three pre-existing CGameDoor
virtual omissions: CompressTime at `0x487530` (slot 0x2C), OnActionButton at
`0x487D10` (0x40), and Render at `0x488B30` (0x4C). The preceding commit
`65a863b4` already lacks those declarations/bodies. This lot adds only a
nonvirtual door helper and does not change those slots. No audit exception,
baseline suppression or placeholder overrides conceal the missing recovery.
Static evidence is therefore recorded as failed despite the successful native
tests. GetMoveDest and MoveToPointRange have GREEN individual parity; the partial
dispatchers and unchanged MoveToObject remain YELLOW. Optional offset analyses
are not analysable for the initially unnamed exports; door/base layout analysis
is also skipped. These skips do not establish equivalence.

Dispatcher recovery remains partial. Static parity and reconstructed runtime
assertions do not establish an original/recovered differential comparison.
Distinct-point door caller behavior and allocation or thread-contention failure
paths are not deterministically covered.

## Continuation

Recover the three missing door virtual methods, starting with the exported
OnActionButton (`0x487D10`). CompressTime and Render need function creation and
fresh exports before recovery; their valid entries and vtable ownership are
established. The following movement executor `0x73EA80` (caller `0x729646`)
builds a formation offset from a resolved sprite and remains unrecovered.

The following [door recovery lot](combat-door-recovery.md) implements those
three virtuals and both polygon helpers. Its refreshed audit exposes the
additional AddEffect override at `0x486860`, now the next door target.
