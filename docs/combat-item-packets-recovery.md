# Inventory message packets

Recovered 2026-10-10: AddItem Marshal/Unmarshal 4F8D60/4F8FA0 and RemoveItem
Marshal/Unmarshal 504070/5041A0. Complete offline contexts and reachable PE
assembly were read before implementation. PE vtables 848BE8/848C04 establish
the message classes and slots 10/14; exported CGameAIArea annotations are wrong.
Shared CMessage header content was already read in the preceding lot; current
declaration changes and both BG2 layout blocks were read here. FUN_7FCFFE is
the MFC CString GetBuffer implementation, confirmed by its full assembly.

AddItem body: target player DWORD, target object LONG, BYTE resource length,
resource bytes without terminator, three WORD charges, WORD wear, DWORD flags.
Size is 21 + resource length. RemoveItem body is the same target pair followed
by a signed SHORT slot, size 10. Marshal writes the caller-provided buffer,
without adding the network header; Unmarshal skips SPEC_MSG_HEADER_LENGTH (3).
The caller/source ID is not transmitted and is retained when decoding.

Marshal retries SHARED/DENIED, shares the target without a type guard, copies
its remote IDs, and releases it before writing. Missing targets set size zero
without changing buffer pointer or contents. Unmapped decode returns FALSE
without changing any message fields or inspecting item/slot payload bytes.
Successful AddItem decode uses a temporary CItem constructor, then SetResRef
and copies only charges/wear/flags. Preserve constructor normalization of
NO_DROP, zero stacked charges and lore/identification flags, and leave existing
ability/sound state untouched. The PE uses a nine-byte resref scratch buffer;
no new packet-length policy is added. This lot does not recover the incoming
network message factory or prove a live multiplayer exchange.

Native profile: scripts/profiles/combat-item-packets.json, centered Save37,
combat-payload fixtures, itempacketchecks before all prior door/combat cases.
Golden 29-byte AddItem packet; resource lengths 0/1/7/8; all three charge
fields; WORD boundaries; raw DWORD flags; signed removal slot boundaries;
120 constructor-normalization cases; unmapped header-only rejection; virtual
Marshal/Unmarshal -> Run -> actual inventory addition/disposal. Resource header,
remote list head, actor IDs, inventory slot, queue and disposal tail restored
in the same tick. No manual timer advance or saved disposable process.

Arc 2026-10-10T10-47-23-084625-batch PASS with four call-count warnings:
clean-first Win32 Debug build PASS; 60-second native observation CLEAN, all
four symbols observed (7/122/9/8 calls). UI PASS: 41 steps, 4347 aggregate
checks, including 272 packet checks. Actual melee/bow/crossbow/sling HP loss
remains 6/12/12/12; projectile payloads remove 3 HP each. Profile cleanup
restored and hashed all preserved paths without errors. Parity is four YELLOW
call counts; provenance's primitive byte additions produce a count mismatch,
with no binary CString operator+ site. Golden bytes confirm operand order.
Optional layout/offset checks are skipped for these message classes. Static
checks, native execution/assertions and differential remain distinct evidence.
Next: restore a bounded part of incoming message routing, or the remaining door
action dispatchers. CMessageHandler::AsynchronousUpdate 4EE020 is still only a
local queue-draining subset of its original network dispatcher.
