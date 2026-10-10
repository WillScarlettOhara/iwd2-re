# Grouped combat effect reception

2026-10-10. Message subtype 105 carries ordered CMessageAddEffect children.

## Scope and binary evidence

- Restore incoming factory 105 in the PARTIAL AsynchronousUpdate `4EE020`: immediate stack decode/Run/destruction, or deferred owned heap message; failed outer decode deletes it. Original branches `4F3428` and `4F34A9`, zero caller/target and empty input list, checked through their shared queue branch.
- Verify existing constructor `5152C0`, destructor `515430`, Marshal `5154E0`, Unmarshal `515670`, Run `5157F0` against full assembly and bundles `tmp_grouped_effect_ctx.md`, `tmp_grouped_effect_lifetime_ctx.md`, `tmp_grouped_dispatch_ctx.md`. Existing source exposes a sentinel argument instead of the original STL list header; callers already build explicit nodes.
- Correct Marshal to retain child updates to its shared buffer pointer across iterations, then advance by each returned size. The outer caller's pointer stays unchanged. Remove obsolete comments saying child serialization is unrecovered; keep function markers adjacent for PDB gate resolution.
- Explicit pack(2), with three padding bytes after field_C: head/count remain at `10`/`14`, total `18`. Compiled four-member spacing PASS. Original final vtable `84D564` confirmed in ctor/destructor; the constructor audit sees the earlier base-table installation.
- `7FC95B` is allocation (malloc/new-handler retry), `7FC984` deallocation; `517000` copies a child pointer. `516FD0`/`448580` construct/destroy the temporary empty list, represented by a stack sentinel in this source. No invented prototypes or helpers.

## Preserved behavior

Marshal body = remote target pair (8), low BYTE count (1), DWORD stride 89 (4), then all child packets. Declared size uses the full count. Missing outer target gives size zero without writing.

Decode skips the network's three-byte header; failed outer mapping leaves target/list unchanged. Otherwise retarget and append BYTE-count children; never clear the old list. DWORD stride controls copying to one scratch buffer after a synthetic header. Outer dwSize is ignored. Child decode failures are ignored and retained with null effects; tests inspect/destruct those children without running them. Empty input still succeeds and retains existing entries. Caller/field_C stay unchanged. Run dispatches children in list order and retains them; destruction owns/deletes each child and all nodes.

No new packet bounds policy or unsupported global dispatcher behavior is introduced.

## Validation

Extend `combat-messages.json`/`.txt`: golden framing and ordered child packets, constructor ownership, virtual pointer updates, repeated Run/destruction, append/empty/failed-child decoding; immediate/deferred MG reception, NULL-provider bypass, mixed FIFO, disconnection, not-ready and unmapped outer target. Retain the transport-thread isolation from the preceding crash fix.

`groupedpacketdamage` receives effects of two and three HP on portrait 1: five HP immediately and five after deferred execution, source unchanged. Existing melee/bow/crossbow/sling, received single damage and opcode-430 damage regressions remain in the same disposable centered save. Never save; profile cleanup restores fixtures/save/config.

Batch `2026-10-10T20-52-41-947760-batch`: clean build 169s, 45 UI steps PASS, 84 packet cases, grouped defender damage 5+5/source zero. CLEAN after 60s hold, 6/6 PDB gates; cleanup restored all paths without errors. Five static warning targets: stale ctor name, raw pointer additions misclassified as CString concatenation, call counts, short Run and PARTIAL dispatcher. Destructor/decode parity GREEN; automatic layout checks skipped, compiled audit above PASS. No differential claim.

Next: incoming factory 117 (on-hit spell metadata; Run `516B70`), then outstanding weapon damage calculations. The global network dispatcher remains PARTIAL.
