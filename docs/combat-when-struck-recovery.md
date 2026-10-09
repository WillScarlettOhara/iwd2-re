# When Struck installer and message 117

Final validation: 2026-10-09T14-52-05-549869-batch PASS with four warning
targets. Win32 Debug was rebuilt completely after the header changes. All
746 checks, 24 scenario steps and nine PDB symbol gates pass, followed by a
60-second CLEAN hold. The defender's HP is 291 -> 285 -> 278 -> 266 -> 254
-> 242 -> 240. Melee deals six, melee plus the installed self effect deals
seven, each ranged shot deals twelve and spends one charge, and direct
on-hit delivery deals two. The attacker loses two HP to the installed melee
riposte and one to direct self delivery. All 46 installer/message checks pass.
Fixture cleanup and preserved save/config hashes pass with no errors.

Static parity is GREEN for the installer, Copy, Unmarshal, Run and on-hit
delivery. The constructor, Marshal, Swing and AddEffect Run are YELLOW for
a short-body heuristic or call counts. The Swing operand-count advisory,
incomplete offset checks and three skipped layout audits remain. A skipped
check is unproven, not binary/runtime equivalence.

The connected lot restores IcewindCGameEffectWhenStruck::ApplyEffect (0x56A5A0)
and message 117's MarshalMessage/UnmarshalMessage/Run
(0x516910/0x516A80/0x516B70). Constructor 0x4A1B40 installs vtable 0x84C010;
its ApplyEffect slot points to 0x56A5A0. The installer constructs the message
inline with vtable 0x84D45C. Its subtype slot reads 117, communication type is
BROADCAST_FORCED_OTHERS and message type is CMESSAGE. CMessage117 follows the
repository's numeric convention; no canonical descriptive class name is known.

Full context inputs: tmp_whenstruck_ctx.md, tmp_whenstruck_message_ctx.md and
tmp_whenstruck_marshal_ctx.md. All were read, including the full CMessage header;
unchanged previously read sprite/effect headers supplied the typed members.
The missing Marshal export was repaired with an address-map anchor and a full
Ghidra create-functions/re-export. That temporary stub was replaced before build.
PE disassembly confirms all field widths, virtual identities, retry loops,
append/replacement behavior and the existing-buffer serialization contract.

ApplyEffect appends one COnHitSpell without setting m_done. It records source
resource, spell resource, target point and the low BYTE of packed casterLevel.
Zero stays zero. Outside SERV_PROV_NULL it sends message 117 with the sprite's
ID as both caller and target. The existing BROADCAST_FORCED_OTHERS handler does
not enqueue that outgoing message locally; with the session closed it deletes
it. Tests cover this branch without opening a network session.

Marshal writes 36 bytes into the caller's existing buffer: remote player/object
IDs, two eight-byte resources, DWORD level and two LONG coordinates. It does
not allocate or replace the buffer. A missing target sets size to zero and
leaves the buffer untouched. Unmarshal begins after SPEC_MSG_HEADER_LENGTH;
an unmapped remote identity returns FALSE before changing any field. A mapped
identity replaces targetId and the payload, leaving sourceId untouched.
As in the PE, valid mapped packets end with a size assertion rather than an
invented preliminary malformed-packet guard.

Run acquires the target, accepts only exact TYPE_SPRITE, clears the entire old
on-hit list and inserts one record. Unlike the installer, it preserves the
full DWORD level. It releases successful non-sprite acquisitions and ignores
missing targets. Contention retry branches remain static evidence; these
same-thread tests do not create real competing locks.

The combat-when-struck profile adds decoded-opcode and Copy checks, append
ordering, levels 0/1/255/256/257/0x112FF, packet bytes and buffer ownership,
mapped/unmapped identity, 0x1000101 packet level, empty/multiple-list replacement,
normal message-drain dispatch and a registered non-sprite target. A temporary
typed remote mapping is removed by restoring the exact prior bucket head;
pending messages are detached and restored around the isolated drain. Actor
lists and the service provider are restored in the same tick.
The local save marks its dummy connection established even with SERV_PROV_NULL.
The test temporarily clears that typed flag and restores it in the same tick;
it never calls OpenSession or CloseSession and never connects to a peer.

The melee retaliation case now installs through DecodeEffect and virtual
ApplyEffect instead of directly inserting a synthetic COnHitSpell. It then
uses Swing and normal effect messages to require six weapon plus one self
damage on the defender and two retaliation damage on the attacker. The effect
is transient test setup, not a saved persistent buff. The scenario repeats
all metadata/action/tick regressions, all three ranged weapons and direct
on-hit delivery, with transactional fixture/save/config cleanup.

The initial batch's re_lint found three missing address markers on the message's
routing virtuals; these were corrected. That run cannot be called a full PASS.
Run 2026-10-09T14-44-51-625514-batch also stopped before any target execution:
its original closed-session precondition rejected the local save's dummy
connection flag. The scaffold now requires SERV_PROV_NULL and saves/temporarily
clears/restores the connection flag instead. The entire batch is rerun.

These tests do not prove live multiplayer reception or matched original/recovered
runtime equivalence. A receiver factory and actual peer session are outside the
lot. The local installer, message wire methods and message drain are exercised
separately; no incoming multiplayer packet is claimed to have arrived.

Next connected effect: IcewindCGameEffectProjectile::ApplyEffect at 0x56A730,
proven by constructor 0x4A1C30 installing vtable 0x84C038 and slot +0x08. It
still lacks a Ghidra function export and a source override; repair its export
before assembling a full recovery context.
