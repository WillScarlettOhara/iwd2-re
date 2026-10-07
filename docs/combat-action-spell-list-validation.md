# SetCurrAction spell-list integration

Extend typed runtime coverage of recovered SetCurrAction (0x7338E0) and
existing FindAvailableSpell (0x727720), without production changes.
Source helpers AddKnownSpell, RemoveKnownSpell, grouped/list Add and the
spell-list declarations were inspected before constructing the fixture.

Twenty-eight cases run all seven spell actions against four isolated list
states: empty lists with priest resource fallback (SPPR101); available mage
spell (SPWI112); available priest spell; available priest domain spell with
class/specialization 0x103 and level unspecified. Expected metadata is fixed
by these fixtures, not calculated by calling FindAvailableSpell first.
Expected level is one. Domain specialization index one must survive.
Available spell current counts remain one: preparing the action must not
consume the spell. Guards keep both resources alive, and exact request and
reference counts must return to baseline with m_curSpell null.

The entire actor spell-list and domain-list structures are copied (vectors
own their entries), cleared between cases, then restored by assignment.
Base specialization is also restored. No save is written and the existing
profile owns save/config backup and restoration. Earlier 365 assertions
remain, for 393 total.

Validation: full batch 2026-10-07T19-21-05-760012-batch passes eight shared
lints, class/layout audits and Win32 Debug build. All 393 typed assertions
and 11 scenario steps pass, followed by 60 seconds CLEAN with both symbols
executed. Fixtures removed and preserved paths restored and hashed.
FindAvailableSpell parity is GREEN; SetCurrAction remains YELLOW (77/64
calls). The helper member-offset audit skips as not analysable, so static
evidence remains incomplete. Missing-resource demand failure and diagnostic
assertion branches remain outside this runtime coverage; no original/recovered
differential equivalence is claimed.

Next: resume combat action execution recovery after the completed
SetCurrAction transition blocks.
