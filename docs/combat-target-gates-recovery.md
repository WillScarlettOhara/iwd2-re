# Scripted spell target gate

This lot restores CGameSprite::CheckSpellTarget at 0x764810. The name is
inferred from its role and marked #guess; no BG2 canonical name was found.
The bridge address map resolves the name in the recovery context. The initial
numeric sub_ name was rejected by RE004, then replaced without adding a lint
baseline exception. The failed first batch is not a passing validation. It
captured the numeric name before its replacement, so its runtime hook was not
instrumented. It also omitted CheckInvisibility from its gate set because the
address marker was too far above the signature for arc's resolver; the marker
is now immediately above the signature. No method behavior changed for that
marker correction. The final batch must exercise all four resolved symbols.

Recovery used tmp_combat_target_gate_ctx.md before implementation, followed by
the resolved final/named bundles. Their shared CGameSprite header and BG2 layout
were identical to the complete blocks previously read in
tmp_combat_hit_spells_ctx.md (matching content fingerprints). Only the new
declaration changes that header. Full PE instructions 0x764810..0x764874
establish the short-circuit order, this/target roles and calls. The earlier
misaligned quick view at 0x72980A is not evidence; the caller instructions
beginning at the valid 0x729835 boundary establish two pushed arguments.

The helper returns FALSE for a dead source, null/dead target, target base flag
0x20000, held source or helpless source. It then calls the source's virtual
GetCanSeeInvisible and the target's CheckInvisibility, finally rejecting target
sanctuary. It reads derived generalState, not base generalState. Held/helpless
targets and source sanctuary remain eligible. The second four-byte stack
argument is unused (ret 8); DWORD models its storage without inventing meaning.

PE also confirms the existing dependencies: CheckInvisibility only hides a
regular-invisible target when the viewer cannot see invisible and the target's
force-visible bit zero is clear. GetCanSeeInvisible permits three dialogue
actions, the derived see-invisible flag or base flag 0x10000. Improved
invisibility alone is not the regular-invisibility bit. Seeing invisible does
not bypass sanctuary in CheckSpellTarget.

targetgatechecks exercises those asymmetries on two live party members, the
ignored argument at 1/2/0x80000000/0xFFFFFFFF, self/null targets, visibility-byte
bits, both see-invisible flags and all three dialogue actions. It restores both
actors' states, flags, spell-state bitsets, source action/vision and target
visibility in the same tick and requires no message emission. The profile
repeats all projectile-opcode combat tests with actual HP/ammo regressions.

This is a recovered query and direct runtime coverage. Its original action and
status-trigger callers are still incomplete in the reconstruction; this lot
does not claim their scripted-casting dispatch is restored. The connected next
function 0x764880 checks spell availability/metadata and remains unrecovered.
Recover it and the original dispatcher blocks before wiring the complete
casting permission path.

Final batch 2026-10-09T17-34-02-836763-batch is PASS with no warning targets:
889 checks, 34 scenario steps, four of four PDB gates and a 60-second CLEAN
hold. CheckSpellTarget is exercised 34 times; all 36 isolated query checks pass.
All four target functions have GREEN parity. The native layout audit checks
310 CGameSprite members successfully. Offset analyses and the effect layout
audit that skip remain unproven, not equivalence evidence.

The initial run compiled every translation unit after the header change; the
final run compiled the final CGameSprite source. Both runs pass the scenario's
HP/ammunition assertions; only the final run has clean lint and complete
instrumentation. Fixture/save/config cleanup restores every preserved hash
without errors. The new four projectile cases retain their three target/zero
source damage, while melee, riposte and all three ranged weapons retain the
previous damage/charge results. No matched original/reconstructed runtime
comparison or restoration of the incomplete caller dispatchers is claimed.
