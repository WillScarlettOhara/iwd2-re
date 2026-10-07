# SetCurrAction spell metadata

Recovered the remaining spell block 0x7340BD..0x734522 in SetCurrAction
(0x7338E0), from the previously read full context bundle and verified PE
instructions. The action whitelist is 31,95,113,114,181,191,192. Resolve
GetString1, or SpellIdToResRef(specificID) when empty. Only GetSpellType==1
continues. Extract class and specialization index from the low two bytes
of specificID2, and level from the low byte of specificID3. If class and
level are both nonzero, preserve the full metadata values unchanged.

Otherwise allocate a temporary CSpell (resource request), search the
actor's available spells, and on failure Demand the resource for caster
type/level fallback: type 1 selects wizard, type 2 cleric, specialization
zero. Release the demand, reproduce diagnostic assertions, pack the class
and specialization index into specificID2 and zero-extend level into
specificID3. Delete the temporary spell and clear m_curSpell, including
the request cancellation/reference release. Existing helper implementations
are reused unchanged. Error string was read from PE at 0x8B87EC.

Twenty-nine typed cases cover all seven spell actions with named and
numeric SPWI112, explicit class/level with high bits that must survive,
unknown resource, and WAIT as a non-spell control. Expected availability
comes from the existing spell-list helper with resource fallback; therefore
these cases verify dispatch/integration, not that helper independently.
The fixture reports whether availability or fallback was used. Checks
cover current metadata, unmodified input action/string and exact resource
request/reference counts under a resource guard. Earlier 336 assertions
remain. Priest fallback, missing-resource demand failure, domain
specialization and assertion paths need further runtime coverage.

Validation: final full batch 2026-10-07T17-49-14-584071-batch passes all
eight shared lints, class/layout audits and Win32 Debug build. All 365
typed assertions and 11 steps pass, followed by 60 seconds CLEAN with
39,927 SetCurrAction entries. Fixtures removed, preserved paths restored
and hashed. Parity remains YELLOW (77 original calls, 64 source calls).
The fixture reports available=false, so resource fallback was exercised;
available-spell integration still needs dedicated coverage.

The first batch passed runtime but failed four CString lint findings;
_T/LPCTSTR formatting and m_resRef member spelling were corrected and all
gates rerun. Static, runtime and differential evidence remain distinct.

Next: extend runtime coverage of available/domain spell metadata and priest
fallback, then resume combat action execution. No full differential
equivalence claim is made for SetCurrAction.

Further coverage: [spell-list integration](combat-action-spell-list-validation.md).
