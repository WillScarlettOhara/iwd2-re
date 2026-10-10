# Received on-hit spell metadata

2026-10-10. Restore incoming CMessage117 (subtype 117) before triggering its
spell through ApplyOnHitSpells and ordinary effect/message ticks.

## Binary scope

Full bundles: tmp_onhit_receive_ctx.md, tmp_onhit_constructor_ctx.md and the
previous tmp_grouped_dispatch_ctx.md. Full reachable assembly checked for
constructor 4F6EF0, Marshal 516910, Unmarshal 516A80, Run 516B70, and dispatcher
factory 4F3AB9 through shared cleanup 4F3BCD. The existing constructor/body and
packet overrides were already recovered; anchor the standalone constructor
address and restore the missing incoming factory in PARTIAL AsynchronousUpdate
4EE020. No invented protocol helpers or other incoming factories.

Important original quirk: blocked TCP reception drops this subtype entirely.
Immediate reception creates a stack message and decodes/runs it. If that
decode fails, retry with an owned heap message: enqueue on success, otherwise
delete. Do not change this to the usual immediate/deferred factory pattern.

Wire body: 36 bytes, remote target pair (8), source/spell resrefs (8 each),
DWORD level (4), signed CPoint (8). Missing marshal target returns size zero
without writes. Decode skips the three-byte special-message header, resolves
the target before reading metadata, preserves caller, and leaves all state
unchanged if mapping fails. No new bounds policy. Run obtains/retries an ASYNC
deny; sprite targets clear the entire on-hit list and install exactly one
record, even with empty resrefs or level zero. Other object types are ignored.

Original final vtable 84D45C checked in assembly. Explicit pack(2), four-member
compiled spacing audit PASS. Constructor audit reports the initial base-table
installation 847B40; the later derived table is confirmed manually.

## Disposable runtime coverage

combat-messages profile centers save 37 and uses combat-payload fixtures.
Golden full eight-byte resrefs, high DWORD level, signed point extrema, missing
target, unchanged failed decode and empty metadata. MG-framed incoming cases:
TCP block values 0/1/255, NULL-provider bypass, FIFO replacement with FFFFFFFF
level, empty record, mixed deferred effect/drop, unmapped immediate failure,
disconnection/reconnection and not-ready rejection. Source/target on-hit lists,
network state, mappings and queues are restored in the fixture tick; local
player ID zero keeps the transport thread asleep during fake sessions.

receivedonhitdamage installs ZRCORIG1/ZRCHIT/level 1/defender point from the
incoming packet. Restore the normal network and ownership before triggering
ApplyOnHitSpells. Require two owned outgoing effect messages, then onhitwait
requires one source HP and two defender HP lost through ordinary ticks.
Keep direct on-hit, melee/bow/crossbow/sling, grouped/single received effects
and opcode-430 projectile damage regressions. Never save the disposable game;
profile cleanup restores files transactionally.

Batch 2026-10-10T21-24-45-825309-batch: clean build 172s, 47 UI steps PASS,
CLEAN after 60s, all 5 PDB gates hit (ctor 18, Marshal 13, decode 13, Run 12,
dispatcher 1881). Received on-hit damage source 1/defender 2; melee 6 and each
ranged weapon 12. Cleanup restored all paths without errors. Four warning
targets: stale exported ctor name, Marshal call counts, pointer additions
misclassified as concatenation in decode, and unsupported PARTIAL dispatcher
scope. Decode/Run parity GREEN; automatic layout checks skipped, separate
compiled audit above PASS.
No original/recovered differential claim. The global dispatcher stays PARTIAL.

Next combat gaps: remaining incoming combat factories, then weapon swing sound
selection sub_756930 (756930), currently a no-op reached by AIUpdate.
