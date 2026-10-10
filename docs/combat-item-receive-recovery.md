# Inventory message receipt

2026-10-10: recovered AddItem constructor 4F5130, RemoveItem constructor
4F5C70 and handler destructor 4F7490; corrected GetDialogReplyReturned's
duplicate address marker to 4F4F60 (GetDialogRequestPending is 4F4F50).
Offline bundles, current shared declarations/BG2 layouts and reachable PE
assembly establish signatures, raw WORD copies and ownership. CBaldurMessage
and CMessageHandler now explicitly use pack(2).

AddItem constructs a fresh CItem, binds its resource with SetResRef(TRUE),
then copies three raw charges, wear and flags. The earlier inline assignment
also copied the ability/sound cache, which the PE constructor does not copy.
RemoveItem keeps the signed SHORT slot and the caller/target LONGs. Handler
destruction deletes non-null queued messages without running them, retains
the list during the virtual destructors, then removes all nodes. 7FB298 is
the implicit MFC list destructor; 516FC0 is an AddTail compiler thunk, not
a new handler method.

AsynchronousUpdate 4EE020 remains PARTIALLY RECOVERED. Restored its local
queue gate (null provider or zero field_4BD4), connection/Peek/Fetch loop,
chat and not-ready settings/version/character replies, and incoming CMessage
factories 2/34. Those factories construct caller/target zero, decode and
either Run immediately or append a successfully decoded heap message. Failed
decode deletes the heap message. Each fetched packet is released; FIFO is
preserved. Host purge/object-control and pending/returned dialogue countdown
follow the PE tail. Countdown uses full-width wrapping and exact TRUE bytes.
The pre-existing Run-held-back recovery guard is retained.

Other ready-player message factories and protocol handlers are explicitly
unimplemented. The full 510-callee dispatcher is NOT recovered; its parity
cannot establish full fidelity. ObjectControl and some not-ready handlers
still call stubs. No live multiplayer exchange or original/recovered
differential is claimed.

Profile scripts/profiles/combat-item-receive.json adds itemreceivechecks to
the prior 41-step combat scenario, centered Save37 and combat-payload fixtures.
Real MG framing -> CNetworkWindow -> Peek/Fetch -> dispatcher -> inventory
or deferred queue. Checks cover both independent immediate gates, nonzero
synchronization blocking, deferred FIFO add/remove, connection bytes 0/255,
not-ready discard, unmapped header-only failures in both modes, raw ctor
charges/cache isolation, destructor ownership and exact dialogue predicates.
The scenario isolates/restores incoming queues, readiness, network flags,
remote mappings, inventory, outgoing messages, disposal, and dialogue state
in the same tick; no manual clock advance or save. Equipment notifications
have a distinct remote owner to avoid outbound network sends.

Arc 2026-10-10T20-00-04-263766-batch PASS with three target warnings.
Clean-first Win32 Debug build PASS (174s); 60-second native observation CLEAN,
all five symbols hit (135/30/2/8/1835). UI PASS: 42 steps, 4382 aggregate
checks, including 35 receipt checks. Melee/bow/crossbow/sling HP losses remain
6/12/12/12; each projectile payload removes 3 HP from the target and zero
from its source. Profile cleanup restored all preserved paths without errors.
Constructor/destructor parity GREEN; RemoveItem constructor and reply getter
YELLOW only for short bodies, checked in full assembly. Dispatcher YELLOW
includes 1065 binary calls versus 40 source calls and a NaN-sensitive lift
warning; these do not prove the absent branches. Optional layout/offset
checks skipped. Static, observed execution and differential remain separate.
Next: extend the incoming factories to combat effect/projectile messages using
their own complete bodies and constructors; retain explicit unsupported paths.
