# SetCurrAction container closure

Recovered the entry gate at 0x7338FD..0x733945 in SetCurrAction
(0x7338E0), using its previously read full context bundle and PE instructions.
An incoming action other than NO_ACTION closes the container only when the
world popup state is 8 and the sprite ID matches CInfGame::m_iContainerSprite.
The original accessor at 0x691130 simply reads world +0xEA4; the recovery
uses the existing typed m_nPopupState field. StopContainer (0x690EB0) is
unchanged. The call precedes the per-action resets and action copy.

Four typed AutoUI cases exercise this gate with a real registered empty
ground container: NO_ACTION, another opener, another popup, and a matching
opener with WAIT. Only the last closes panel 8 and changes the popup state
to -1. Every case checks the resulting current action and empty item list.
The temporary container is removed; container IDs, popup, panel activity,
view and auto-hide state are restored. No save is written.

SetCurrAction remains partial, including modal-state, messages, target and
spell resource transition bookkeeping. This arc does not establish full
original/recovered differential equivalence.

Initial run .arc/2026-10-06T19-07-43-486196-batch passes all 120 typed
assertions and eleven steps, observes CLEAN for 60 seconds and hooks both
targets (41380 SetCurrAction calls; one StopContainer call). Its overall
FAIL is a comment-address lint finding, fixed by describing the gate in
prose instead of starting the comment with an interior instruction address.
The fixture then preserves the exact pending view-resize fields.

Run .arc/2026-10-06T19-10-45-692229-batch passes source, class and build
checks but cannot launch runtime: Windows denies transaction.tmp replacement.
The profile journal confirms restoration with no cleanup errors. This run
is not final runtime evidence.

Final validation: .arc/2026-10-06T19-12-39-129027-batch, PASS with two
warnings. Eight source checks, class audits and Win32 build pass. All 120
typed assertions and eleven UI steps pass. CLEAN observation for 60 seconds;
SetCurrAction hits 41336 times and StopContainer once. Profile paths are
restored and hashed. Parity remains YELLOW (77/11 and 17/24 original/source
calls); one layout audit is skipped. Full static and differential equivalence
remain unproven.

Next target: SetCurrAction modal-state transition before the action copy.
