# Combat target tracking — 2026-09-25

`CGameSprite::UpdateTarget` (0x733660, virtual slot +0x70) previously called the
empty base implementation. It now tracks the target ID and updates sprite target
markers as in the original executable. `ResolveActionTarget` calls it twice on
its successful path, so the same-ID early return matters: each targeting actor
must contribute only one marker reference.

## Binary evidence

Full recovery context and provenance: `tmp_update_target_ctx.md` and its manifest.
The CGameSprite vtable is 0x85BDD4. Assembly 0x733660–0x7337C5 establishes:

- A non-null target with the existing ID returns immediately.
- The old target is acquired with GetDeny on THREAD_ASYNCH, retrying SHARED and
  DENIED. Any other failure returns without replacing or clearing the old ID.
- For a selected, orderable attacker, a previous sprite receives ELLIPSE and a
  new sprite receives RECTICLE through CMarker::SetType (0x7670D0). Its counter
  allows several attackers to share a marker.
- A new sprite's force-render byte is set to 15 only when actionCount is zero
  and userCommandPause is strictly greater than 73 (five times 15 minus two).
  The signed SHORT comparison at 0x73379F/0x7337A1 excludes exactly 73.
- Other object types still become the tracked target. NULL clears the ID after
  successful release of any previous target.

## Tests

`combat-target.json` centers the camera on the loaded party and runs fifteen
target checks followed by fourteen direct GroupAttack and seven ExecuteAction
dispatch checks. Target cases cover duplicate calls, switching, clearing,
marker sharing, pause boundary, nonzero action count, unselected/unorderable
attackers, and failure to acquire the previous object. The harness restores
target IDs and both markers as well as the existing action/AI snapshots before
the next tick; it writes no save.

The optional `--target` mode of `group_attack_reference.py` runs the same fifteen
target checks on the original, then the fourteen direct GroupAttack checks.
`update_target_reference.js` restores its field snapshots in a finally block.
The seven dispatch checks are excluded from this comparison. Captures compare
named boolean predicates and their order, not all actor state. Lock contention,
non-sprite targets and successful locks yielding NULL are not runtime-tested.

## Validation

Run `.arc/2026-09-25T17-38-41-882078-batch/`: VS2019 Win32 build, eight source
checks and class/layout audits passed. All 36 assertions and eight scenario
steps passed. The 60-second observation was CLEAN: UpdateTarget 15,939 entries,
GroupAttack 10, ExecuteAction 93,330 (all world actors combined). Profile
cleanup restored preserved paths and checked their hashes.

Static parity remains YELLOW: UpdateTarget reports 8 binary calls versus 14
source calls (including accessors), GroupAttack 24/9, and the still-partial
ExecuteAction 284/70. The UpdateTarget and GroupAttack offset audits are not
analysable. These results do not establish complete binary equivalence.

Original runs `.arc/2026-09-25T15-43-19-023915-group-reference/` and
`.arc/2026-09-25T15-44-56-543746-group-reference/` failed cursor calibration
before loading the save or reaching any check. They remain UNPROVEN; no target
differential result is claimed. The prior fourteen direct GroupAttack matches
documented in `combat-group-reference.md` do not validate UpdateTarget.

On 2026-10-03 the loading failure did not recur: all fifteen target predicates
and fourteen direct GroupAttack predicates matched the original, followed by
the seven enemy-dispatch predicates. See `combat-dispatch-reference.md` for the
29/29 and 36/36 captures and their limits. The earlier failed runs remain retained.

Next useful work: compare damage under matched combat inputs.
