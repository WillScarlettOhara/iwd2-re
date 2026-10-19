# Character popup cancellation and reverse level-up flow

Targets: IsDoneButtonClickable 5E1F50, RewindLevelUp 5E1B20 (#guess),
OnCancelButtonClick 5E3B80 and CAIScript::SetResRef 514220.
Bundles/manifests: tmp_finishlevelup_context.md and
 tmp_finishlevelup_dependencies_context.md; full headers and ASM read.
Canonical MD5 25cb3d8aa2ec648e94e328d8c467973e.

Done gates preserve signed point/index comparisons, specialization masks,
ranger-slot bytes and export-name trimming. Reverse order:
16->8->56->55->7->6->54->2. Leaving specialization restores base/derived stats,
preserves HP/base max/derived max/CON contribution across effects and retains
an acquired death flag. Terminal rewind dismisses the stack and clears readiness
without completing a level; host BYTE 0/1/other remain distinct.

Cancellation locks UI/actor, retries SHARED/DENIED and releases the original ID
before invalidation. It restores specialization, abilities, skills, feats,
favored enemies, spell banks, portraits, sounds and scripts. Panel 54 writes
specialization through the global character engine, restores AI/stats and deletes
three snapshots. Panel 8 clears nonempty temporary spell vectors and shared
max/total, retaining shared current; empty-list aggregates survive. All seven
class banks and domains restore by deep assignment. Sound copying uses CString
allocation capacity, capped at 32; only capacity below 32 adds a terminator.
Padded short strings retain copied tail bytes. SetResRef forwards ECX to CResRef
assignment with a hidden return buffer: it updates the script despite the
misleading decompile. CAIScript explicitly uses pack(2).

Reference: scripts/probes/cancel_levelup_reference.py;
tmp_cancel_levelup_native9/report.json PASS: 308 numeric gates, 13 text cases,
756 reverse flows, 198 cancellation receipts and four exact script references.
Code/globals restored; no callback errors; private PID retired. Gates, MFC trims,
skill/feat setters and SetResRef execute unchanged. MFC allocation, UI, locks,
object lookup, effects, stats lifetime/assignment, script loading and bank
assignment are explicit doubles; receipts do not prove those callees.

Typed CancelLevelUpChecks replays 321 gates and checks every panel, failed lookup,
real reverse popups, deep spell copies, aggregate quirks, HP/death, deletion,
portraits, sound capacity/padding and scripts. Skills are checked before rewind
can overwrite them. Actual GUIREC entry/Cancel controls dispatch both handlers.
Stats, AI, spell banks, owned script, sounds, engine/network and clock restore
in one tick. Effects defer; original doubles perturb HP. Full re-derivation is
unproven. Typed script restore uses an empty resource. No save/equipment copies.

Failed runs exposed two fixture races: fake provider state reached null DirectPlay,
and the renderer used the private engine. Both fixtures now pause transport until
restoration, route controls via m_pEngineCharacter and never replace pActiveEngine.
A projectile false negative exposed reused addresses; identify launches by object ID.

ARC 2026-10-19T11-52-35-361507-batch: PASS with warnings, BUILD OK, CLEAN,
90/90 steps, 787 checks/60 cancels. Hits: gates 321, rewind 16, cancel 62, setter 7.
Cleanup restored hashes. cancel_levelup_compare.py: MATCH for 321 ordered gate
events, no tolerances/ignored fields (op->tag only); artifacts in run's
cancel-levelup-pair/. Other behavior is outside that differential proof.
Static evidence remains unproven: skipped layout/offset checks, call-count and
short-body warnings. Source lints/class audits pass.

HP prerequisites 542A90/542B40/542CB0/542FA0: see hitpoints-recovery.md.
Next: level-stat helper 5D8EA0, then OnDoneButtonClick 5E2210. Its native
HP-cache-hit branch reads an uninitialized local; do not invent deterministic HP.
Skills reset 5F7850 and feats reset 5DA200 remain incomplete. Keep the full profile.
