# Door toggle and local inventory delivery

Recovered on 2026-10-10 from complete offline contexts and PE assembly:

- ToggleDoor 489680: locked/forced access, party/personal/bag keys, special
  first-opening refusal, occupied-close orders, positioned sound, search masks,
  tile/destination updates, status messages, trap triggers and consumed keys.
- DoorOpen::Run 4FD1D0: deny target, shared source, exact raw BYTE comparison,
  missing IDs and source AI selector delivery without a target-type guard.
- FindDoorOccupant 47A1C0 and IsOverDoor 54A210 (names guessed): alternating
  previous/next front-list search, active sprite guards and held share returned
  to caller; fixed 320 coordinate limit, signed radius and asymmetric clipping.
- TakePartyItem 463B30 and PlaceItem 464950: personal/bag transfers, all/number/
  squared-range variants, quick-button invalidation, stack remainder messages,
  recursive copies, familiar/protagonist and container/ground placement.
- AddItem::Run 4F9150 and RemoveItem::Run 504250: local inventory changes,
  equipment/status delivery, disposal and quick-slot/weapon-set updates.
- UpdateQuickWeaponData 726F20 (name guessed): four paired weapon sets,
  selected ability/ammo matching, list ownership and equipment notification.
- Corrected FindItemPersonal 71D310: compare the uppercase copy and return -1
  for an absent item, as the PE does. This fixes false key possession and an
  invalid item dereference in the transfer path.

Door now owns its missing CSound at 676. Door, search, area, item, sound and
the new message mirrors use explicit pack(2). Item member spacing and its
unrounded SHORT tail are checked; absolute base-layout gaps remain unresolved.

Preserved quirks: opening uses ClosedDest and closing OpenDest; DoorOpen BYTE
255 requests a toggle; occupancy share failures/non-sprites skip the opposite
side that iteration; non-party direct placement uses slot index+15 while the
deny-acquired path writes index+18. Failed/full ground fallback returns FALSE
even after placing. Familiar delivery leaves the original pointer with caller.
Bag-only TakePartyItem can return ERROR despite taking an item. Partial-stack
transfer copies the remainder into both delivery paths. No clock advancement.

Validation: scripts/profiles/combat-door-toggle.json and its scenario run the
new isolated doortogglechecks before the existing door effects and combat
regressions. Save37 is centered before tests. Synthetic door/actor state,
search cells, inventory pointers, quick slots, queued messages and disposable
items are restored in the same tick; the process is never saved.

All-symbol arc 2026-10-10T10-29-05-257206-batch: clean-first Win32 Debug build
PASS; 60-second native observation CLEAN, all 10 symbols exercised. UI PASS:
40 steps, 4075 aggregate checks, including 218 new toggle/inventory checks.
Real HP loss: melee 6, bow/crossbow/sling 12 each; projectile payloads 3 each.
Profile cleanup restored and hashed all preserved paths, with no errors.
Static parity: four GREEN, six YELLOW call counts; provenance has one count
warning. Optional static checks remain skipped; this is not differential proof.
Standalone CItem/CSound member-spacing audits also PASS.

Limits: AddItem/RemoveItem packet serialization and incoming network factory
dispatch remain inherited/unrecovered. This lot restores local message Runs;
it does not establish multiplayer fidelity. Static evidence is not an original
versus recovered runtime differential. Follow with the four item message
Marshal/Unmarshal functions (4F8D60, 4F8FA0, 504070, 5041A0), then the remaining
door action dispatchers as a separate coherent lot.
