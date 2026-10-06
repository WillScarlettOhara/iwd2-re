# SetCurrAction modal transition

Recovered 0x7339A4..0x733BEB in SetCurrAction (0x7338E0), using its
previously read full context bundle and PE instructions. The binary's 27
unique exempt action IDs preserve a nonzero modal state. Other actions call
SetModalState(0, TRUE), after per-action resets and before marker resolution.
The existing SetModalState implementation supplies feedback, stealth exit
and toolbar updates; it is unchanged.

When the old mode is 2 (search) or 4 (turn undead), clear the selected
action-bar button to 100. GetCharacterPortraitNum precedes the active
engine's GetSelectedCharacter call. For the same selected portrait, call
ResetState unless the current bar state is one of the 16 literal exemptions
in the original. The adjacent innate-picker state 0x6B is not exempt.
Original offsets 0x35F6/0x35FA map to the existing button array's selected
button/state fields, at 0x1C78 + 0x197E/0x1982.

105 new typed AutoUI assertions: every exempt action in modes 2, 3 and 4;
ATTACK from modes 0, 2, 3, 4 and 5; modes 2 and 4 with another selected
portrait; each protected toolbar state; and the nonprotected 0x6B state.
Checks cover modal state, hiding flag, incoming action, toolbar state,
selected button and stack consumption. Selection uses the world's real
group-leader lookup; the fixture restores the original ordered group list.
Modal/hiding/portrait/bar state is restored, queued fixture messages are
deleted, and no save is written.
The existing 120 assertions remain in the same disposable scenario.

SetCurrAction remains partial: action-change messages, selected target
release, temporary weapon selection and spell/resource bookkeeping are
not fully recovered. SetModalState and ResetState are runtime gates for
this call routing, not newly recovered bodies or full equivalence claims.

Initial .arc/2026-10-06T21-18-19-799952-batch fails compilation: the
existing out-of-line action constants cannot be switch case expressions.
The same whitelist is expressed as a typed SHORT array. No runtime claim
is made for that failed run.

Final .arc/2026-10-06T21-20-55-054841-batch: PASS with two warnings.
Eight source checks, class audits, sprite layout audit and Win32 build pass.
All 225 case assertions and eleven UI steps pass; CLEAN observation for 60
seconds. SetCurrAction hits 41462 times, SetModalState 23, ResetState four.
Profile paths are restored and hashed. Parity is YELLOW for SetCurrAction
(77/17 calls) and SetModalState (53/43), GREEN for ResetState. Offset analysis
is unavailable for SetModalState/ResetState; the button-array layout audit
skips because it does not recognize the header's bare hexadecimal comments.
Full static and original/recovered differential equivalence remain unproven.

Next target: SetCurrAction temporary weapon selection and trap-mode messages.
