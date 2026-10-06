# SetCurrAction weapon snapshot and trap messages

Recovered 0x733C14..0x733CCD within SetCurrAction (0x7338E0), from its
previously read full context bundle and PE instructions. Except for action
92 (the SHORT at 0x84782C), incoming actions snapshot the selected weapon
and the low BYTE of its WORD ability index into the existing temporary
selection fields. No equipment selection is changed.

Before copying the incoming action, enqueue CMessage107(TRUE, id, id) for
REMOVETRAPS. This includes REMOVETRAPS-to-REMOVETRAPS, not only entry from
another action. When leaving REMOVETRAPS, enqueue the same message with
FALSE. The original construction installs vtable 0x84D424, identified by
its Run entry 0x515B00 and subtype getter. Dispatch uses AddMessage(FALSE).

The message's Run is also corrected from GetShare/ReleaseShare to the
original GetDeny/ReleaseDeny. Its full bundle and PE instructions show an
ASYNC deny lock, SHARED/DENIED retry, sprite-type gate, BYTE zero extension
into field_580, and release on every successful lookup. Existing names and
the remaining body are unchanged; no new address-map entry is needed.

14 new typed assertions: seven action IDs check the snapshot, action-92
exception, neighboring action 93 and truncation of 0x1234 to 0x34 without
changing equipment; five transitions check message count, subtype, caller,
target, payload, delayed state write and execution; direct messages check
BYTE 255 and invalid-target no-op. Fixture state and temporary weapon
fields are restored, queued messages are deleted, and no save is written.
The scenario retains its earlier 225 assertions.

SetCurrAction remains partial, including action-ID notifications, target
marker reservation release and spell/projectile resources. No full
original/recovered differential equivalence is claimed.

Validation: .arc/2026-10-06T21-30-47-054750-batch, PASS with two warnings.
Eight source checks, class audits, sprite layout and Win32 build pass.
All 239 typed assertions and eleven UI steps pass; CLEAN observation for
60 seconds. SetCurrAction hits 41497 times and CMessage107::Run six.
Profile paths are restored and hashed. Parity remains YELLOW (77/27 and
3/7 original/source calls). Message offset analysis is unavailable; its
layout audit skips because it does not recognize bare hexadecimal header
comments. Full static and differential equivalence remain unproven.

Next target: SetCurrAction current-action-ID notifications after action copy.

That notification is subsequently recovered in
[combat-current-action-notification-recovery.md](combat-current-action-notification-recovery.md).
