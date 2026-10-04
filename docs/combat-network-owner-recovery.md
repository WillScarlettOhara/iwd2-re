# ProcessAI network ownership return

Recovered the missing early return at 0x72C247..0x72C268 inside ProcessAI
(0x72B9A0), using the shared full context bundle and original instructions.
The DWORD at Chitin+0x96E is CNetwork::m_nServiceProvider (network starts
at 0x952, field at 0x1C). Chitin+0x104C is m_idLocalPlayer (0x6FA),
compared against sprite m_remotePlayerID at 0x62.

Nonzero provider plus unequal owner returns after animation/selection
bookkeeping and before dialog cancellation and action processing. Null
provider always continues. The branch does not query GetSessionOpen or
hosting. The poison counter and movement override
remain before this return. Removed a misplaced ProcessEffectList call at
source entry: original 0x72B9A0..0x72BBC3 contains no such call. Its two
actual sites (detached dead routing and later action gating) remain intact.

Four typed AutoUI cases synchronously exercise null/TCP provider and
equal/unequal owner. An escaping actor's dialog wait is the downstream
canary: it clears in three cases and remains 100 for a foreign TCP owner.
The poison sound counter decrements from five to four in every case,
asserting the position of the return. World time is stopped. Provider,
local/remote identity and escaping/dialog fields are restored before
the next tick; no connection is opened and no save is written.

The first run .arc/2026-10-04T22-08-42-235761-batch failed the foreign-owner
counter assertion: the misplaced entry call returned before decrement.
Dialog wait alone would have missed that error. No successful runtime
verdict is attributed to this run.

Final validation: .arc/2026-10-04T22-11-52-294105-batch, PASS with two
warnings. Eight source checks, class/layout audits and Win32 build pass.
All 86 case assertions and eleven UI steps pass, followed by CLEAN
observation for 60 seconds; both ProcessAI and ResolveInstants are hit.
Save/config paths are restored and hashed. Parity remains YELLOW (191/238
and 17/35 original/source calls), and offset analysis is unavailable.
This fixture proves the local conditional routing;
it does not prove a real multiplayer session or full ProcessAI equivalence.

Next target: missing multiplayer visibility and enemy/ally bookkeeping
at 0x72BAD7..0x72BB9C, before the counters.
