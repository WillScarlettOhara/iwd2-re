# MFC list target review — 2026-09-05

Reviewed against canonical `.bin/IWD2.exe` (MD5
`25cb3d8aa2ec648e94e328d8c467973e`) using `fn_digest.py --full` and
`sym.py disasm`. These four entries were incorrectly promoted by recovered
`CGameSprite` caller consensus. Calling a container does not establish that
the container implementation belongs to the game.

| Address | Export name | Reviewed role | Assembly evidence |
|---|---|---|---|
| `0x727BF0` | `FUN_00727bf0` | Typed pointer-list constructor | Calls `0x7FB239` with the block-size argument, installs vtable `0x85C078`, returns `this`. The callee zeroes list head/tail/count/free nodes/blocks at `+4/+8/+C/+10/+14`, stores block size at `+18`, and installs base vtable `0x85F9A0`. |
| `0x727CD0` | `FUN_00727cd0` | MFC pointer-sized list `AddTail` | Allocates 12-byte nodes through `0x7FB7A8`, links the free-node chain, increments count at `+C`, writes value at node `+8`, updates tail `+8` and empty-list head `+4`, returns node/POSITION. |
| `0x727D50` | `FUN_00727d50` | MFC pointer-sized list `RemoveAt` | Unlinks both neighbors, repairs head/tail, pushes node to free chain `+10`, decrements count `+C`; when empty, clears list state and calls block-chain freeing routine `0x7FB7C8`. |
| `0x727DC0` | `Find` | MFC pointer-sized list `Find` | Starts at head `+4` for null position, otherwise at position's successor; compares pointer-sized value at node `+8`, follows next at `+0`, returns matching position or zero (`ret 8`). Ghidra's library annotation also reports matching `CObList::Find` / `CPtrList::Find` VS1998 implementations. |

The exact template element type of the three list operations is not established
by these machine instructions; no concrete C++ template name is assigned.
The constructor's five callers build spell, domain, innate, song and feat button
lists. The other helpers perform only the container mechanics above, with no
gameplay branch or callback. This is enough for library ownership classification,
not a claim of recovered game behavior or runtime parity.

`next_targets.py` records these address-specific reviews with the original
export name as a guard, preserving that name and the caller counts in JSON.
They remain visible in the excluded queue, with this evidence link. No source
address marker is added and the recovered-function metric is not increased.
An export renamed at one of these addresses falls back to ordinary classification.

## Next game target

`0x727720` remains a genuine game candidate: its six callers include
`CGameAIBase::ForceSpellAction`, `ForceSpellPointAction`, `FireSpell`,
`FireSpellPoint`, `CGameSprite::SetCurrAction` and unrecovered `0x75C980`.
The context bundle shows a search for an available spell entry across class
and domain spell lists, producing class, domain/specialization and spell level.
The decompiler's types and boundary conditions still need assembly verification
before implementation. This is identification evidence only; no recovery or
dynamic test of that function has been completed in this review.
