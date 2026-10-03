# Morale check recovery

2026-10-03. Recovered CGameSprite::CheckMorale at 0x71CFF0 using the complete
tmp_morale_check.md bundle, BG2 PDB method name and PE disassembly through
0x71D0AE. The binary makes no calls; game/save accessors inline in our source.

The morale byte is baseStats.field_25C (sprite +0x808), not the separately
named baseStats.m_morale. Immunity is bit 0 of baseStats.field_2F0[3]
(sprite +0x89F). Immunity returns before any update. Otherwise morale clamps
to 20; zero recovery time resets it to 10. Nonzero recovery moves it one step
toward 10 whenever unsigned frame % (DWORD recovery * 15) is zero.
Cutscene and action flag bit 0 exempt the threshold check after updating
morale. The unsigned threshold comparison is strict: equality fails.

AutoUI combatmorale exercises 14 bounded cases synchronously on a party
actor and restores morale, immunity, threshold, recovery, frame, action flags
and cutscene before returning to the engine. Cases include unsigned negative
frame arithmetic, a DWORD_MAX threshold, recovery phase, clamping, equality,
immunity and both exemptions. The profile centers the party and preserves
save/config files transactionally, without requiring weapon fixtures.

At this initial checkpoint the function was not yet called by ProcessAI.
The subsequent [morale failure recovery](combat-morale-failure-recovery.md)
connects it to entry, panic/flee/berserk selection and shared exit. The subsequent
[dialogue pause recovery](combat-dialog-wait-recovery.md) restores its countdown
and idle/facing/path messages.

Validation: .arc/2026-10-03T21-15-13-882267-batch, PASS with one skipped
static check. Parity GREEN; eight source checks and class/layout audits pass;
Win32 build succeeds. Runtime CLEAN over the 60-second hold, CheckMorale hit
14 times, all 14 cases and eight UI steps pass. Profile cleanup restored and
hashed preserved paths. The offset oracle cannot analyze a function with no
thiscalls; this skip is retained, not described as a passing offset audit.
This is bounded runtime coverage, not original/recovered differential proof.
