# Non-PC ammunition fallback — 2026-09-21

This follow-up tests the recovered CheckItemDepleted -> SelectLauncherAmmo ->
EquipMostDamagingMelee path. It changes only the test harness, not combat rules.

The original IsPC (0x5851F0) compares GetAIType().m_nEnemyAlly to EA_PC; this was
checked directly in PE assembly. After the normal party attack click and before
the last round is spent, mode 3 temporarily changes that classification to
EA_ALLY. It restores the original byte immediately after checking depletion,
including the timeout/failure path. Party membership itself is unchanged.
This isolates the non-PC branch; it does not reproduce a complete NPC AI actor.

The fixture offers a stronger melee weapon in slot 45 (score 1d4+1 = 5) and a
weaker later candidate in slot 47 (score 0), with one ranged round in slot 11.
The expected non-PC result is weapon set 1 / slot 45. A PC control with the
same available melee equipment must still select fists (slot 10). Each case
restores equipment and quick slots; the profile preserves save/config files.
No hit roll or combat frame is forced, and damage is not an assertion here.

The first attempt `.arc/2026-09-21T23-15-06-726677-batch/` built but failed
before runtime: Windows denied replacing transaction.tmp with transaction.json
during fixture installation. Cleanup recorded restored=true and no errors;
no fixture files remained. That report is retained as failed preparation,
not runtime evidence.

The retry `.arc/2026-09-21T23-17-42-683911-batch/` passes all 93 UI steps.
For each of bow, crossbow and sling, the non-PC branch selects slot 45/set 1
and the PC control selects fists despite having both melee weapons available.
The attack action is already 0 at each observed transition: selecting the
melee weapon does not itself continue the original attack order. All three
classification restorations report EA_PC (2), and each equipment restoration
passes. These results cover six last-round shots.

The VS2019 Win32 build passed, followed by a CLEAN 120-second observation.
CheckItemDepleted executed six times and EquipMostDamagingMelee three times.
Profile cleanup removed the fixtures and verified hashes of restored preserved
paths, including the selected save. Shared source, class and layout checks passed.

No original/reconstruction paired capture is attached. EquipMostDamagingMelee's
offset audit is non-analysable; call-count parity is YELLOW for both targets.
The next useful comparisons are an actual NPC scenario, same-score weapon
ties and alternative ammunition resources; the classification test alone
does not establish those behaviors.
