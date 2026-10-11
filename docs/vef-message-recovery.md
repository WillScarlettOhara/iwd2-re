# Trailing VEF messages

Lot: constructor 4F69B0, Marshal 511D00, Unmarshal 511F40, subtype-86
factory in the still-partial AsynchronousUpdate 4EE020. Both projectile Fire
callers (52C050/5791D0) use the recovered constructor signature.

Evidence: full bundles tmp_vef_message_ctx.md, tmp_vef_ctor_ctx.md and
tmp_vef_dispatch_ctx.md; complete serializer/constructor disassembly and
the dispatcher branches at 4F1FB1..4F211C. Vtable 84D328 slot +18 points
to 799CA0, whose sole instruction is ret. No remote controller or animation
creation is hidden in Run. Subtype 86 is removed from our Run holdback list.

Constructor: native argument order is resource reference, area CString,
origin by reference, target CPoint by value, SHORT height, caller, target;
ret 20h confirms eight stack words. Copies all fields, retaining caller and
target. Its standalone body was previously unnamed; projectile constructors
were also copied inline. Pack(2) mirror remains 2A bytes.

Payload (specific-message header excluded): remote player DWORD, remote
object LONG, resource length BYTE and resource bytes, area length BYTE and
area bytes, launch X/Y LONGs, target X/Y LONGs, height SHORT. Size is 28 plus
both full CString lengths. Marshal retries SHARED/DENIED; a failed GetShare
still writes the complete payload with player zero and INVALID_INDEX.
String buffer access at 7FCFFE is CString::GetBuffer, not a resource loader.
An area length above 255 wraps only its prefix: all bytes are still emitted.

Unmarshal starts at SPEC_MSG_HEADER_LENGTH. Find success sets the local
target, failure sets INVALID_INDEX but decoding continues and returns TRUE.
The source stays unchanged. Resource assignment from CString uppercases
the VEF name; area assignment preserves case. Native temporary CString and
CAIObjectType construction is retained. The declared size is ignored.
Native trusts lengths and copies into two nine-byte local buffers; this
recovery retains that limitation. Fixtures never decode oversized names or
read beyond allocated packet storage; short declared sizes use full backing.

Reception: native subtype 86 follows the ordinary immediate/heap-deferred
factory, unlike message 117's retry-only branch. Deferred messages retain
unmapped targets and FIFO order. Unblocking drains Run and deletes messages;
Run is faithful no-op. Remaining protocol factories are still unimplemented.

AutoUI vefmessagechecks compares independent little-endian golden bytes for
all 81 resource/area length pairs 0..8, signed coordinates and SHORT heights,
virtual overrides, missing targets, ignored declared size and a 256-byte
Marshal-only area. Real MG frames cover immediate/deferred execution,
complete queued fields, unmapped FIFO, not-ready and exact-TRUE connection
gates. A derived observer proves subtype 86 reaches the local Run drain.
All temporary mappings/network queues, actor IDs, windows and readiness are
restored in the same tick; area queues and clock remain unchanged. The full
combat profile also retains VEF loading and actual weapon HP regressions.

Validation: .arc/2026-10-11T12-31-58-024725-batch, full five-target profile:
PASS with two call-count warnings (Marshal 9/16; existing ICE Fire 35/49).
Three GREEN/two YELLOW; eight shared source checks and three class audits
PASS. Offset oracles remain not analysable, two layout checks lack sufficient
annotations, and there is no original/reconstruction differential capture.
Win32 Debug build PASS (175s); all 268 new checks, 88 VEF-load checks and 52
UI steps PASS. Hits: ctor 187, Marshal 84, Unmarshal 94, base Fire 36, ICE Fire 7.
60-second hold CLEAN; transactional cleanup PASS. Actual Swing HP loss:
melee 6; arrow/bolt/sling 12 each. No save was written.
Next: locate VEF event-to-animation creation; do not invent it in message Run.
