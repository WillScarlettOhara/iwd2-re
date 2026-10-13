# Area selection and live chain damage

Recovered FindObjectNear 46DAE0 and corrected GetNearest 46B160. Full bundles
tmp_nearest_ordinal_context.md/tmp_find_near_context.md, manifests/header read;
native bodies through 46BAC9/46DDB2 reviewed. Argument types retained.

Selection mode is not an ordinal. Native front-list nonzero modes seed best
Y-distance to zero and reject ordinary candidates before farthest update.
Preserve this quirk. Chains can make multiple hops if each nearest is unseen.

GetNearest re-reads virtual position after initial validation; signed-WORD
tile wrapping retained. Out-of-range tile allocates/formats/frees 128 bytes;
negative initial points do not. Source share released before scanning.
Backward/forward native passes preserve ties; failed backward share retries
backward before forward, while a null candidate node skips without pruning
that direction. Products/shifts/subtractions wrap DWORD before signed LONG
comparisons/division. Existing sprite state, invisibility, immunity, stealth,
race BE, active flags, source visibility and explored LOS gates retained.

FindObjectNear scans front list from head. Negative center returns invalid.
Mode 0 seeds signed SHORT range squared+1; nonzero seeds zero, only exact 1
updates farthest. Scale center/candidate Y with wrapped *4 /3. Race BE filtered
before vertical band checks. Above-band entries skip; first eligible below-band
entry releases share and returns accumulated result. IsOver receives SCALED
candidate Y. Sprites require all three active flags, above-ground animation,
CheckInvisibility; non-sprites bypass these. OfType(FALSE,FALSE). Any nonzero
includeAll requires general-state bit 10000000 even for non-sprites (unchecked
native cast retained). CheckLOS uses original world positions/explored FALSE.
Distances wrap and compare signed; ties retain first, farthest only clips Y.

Native area_nearest_reference.py PASS49, canonical MD5, report in
tmp_area_nearest_reference. Original never resumed; PID killed, hooks restored,
fingerprints stable, cleanup clean. Type/lock/animation/visibility/LOS callbacks
isolated: list/scalar proof only. Golden header from captured IDs.

AutoUI nearestchecks uses a private typed area/front list and four real party
sprites; 49 captured selections replay with remapped IDs, scaled point filters,
equal-point LOS and nonfront delegation. The original fixture used explicit impacts;
the launch continuation now runs factory/Fire/AI arrival and two children
with real ZRCHIT copies and owned grouped messages. Six HP per defender, instant effects
not retained. Restore full actor base/derived/temp stats, positions, list nodes,
flags/damage metadata and area globals; retire all fixtures before next tick.
Chain fixtures share the private area and temporarily clear AI-type immunity;
snapshots restore the original area and deep-copied immunity lists afterwards.
Private visibility buffer detached before area destruction; no save written.
Existing beam/chain/weapon regressions retained.

First .arc/2026-10-13T17-35-11-675718-batch ERROR: all 49 selections matched;
private search grid unset, so chain LOS rejected and HP stayed unchanged.
Build passed, no crash, UI-FAIL abort. Fixture now seeds dimensions and uses
three decreasing spacings within one search cell. Cleanup restored hashes.
Second 17-41-26-631531 ERROR: first defender lost six HP; later target shares
still returned original area, so search used its visibility. Fixture actor
area pointers now join the private area and restore with their snapshots.

Final .arc/2026-10-13T17-47-45-091065-batch PASS: Win32 Debug, 70 UI steps,
66 checks/49 golden cases, three defenders -6 HP each, 60s CLEAN. PDB nearest/
point/chain hits 23912/34/15. GetNearest/chain parity GREEN; point YELLOW calls
14/19. Missing offset/area-layout oracles leave full static equivalence
unproven. Cleanup restored hashes; no gates reduced. Weapon/beam checks pass.
Initial factory launch recovered; see [continuation](chain-launch-recovery.md).
