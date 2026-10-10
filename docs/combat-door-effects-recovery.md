# Door effect delivery and summon visual

CGameDoor::AddEffect (0x486860, slot 0x78) handles Knock, DetectTraps,
CastSpell and Summon; it deletes effects and discards other opcodes.
List/noSave/immediateApply are ignored.

Knock clears lock bit 2 and queues CMessageDoorOpen(TRUE). Difficulty exactly
100 instead displays strref 0x5A81 and preserves flags. DetectTraps requires an
active trap. New detection requires unsigned WORD difficulty <100 and flag 8;
messages are SetTrigger(DETECTED), TriggerStatus, optional SetDrawPoly(400),
DoorStatus. Existing detection refreshes the flash; the reentrancy byte blocks
only its message. Original AI equality and auto-pause logic are retained.

CastSpell sends ForceSpell to the effect source, targeting the full door AI
selector, preserving resource and signed amount. Summon loads a CRE, finds a
passable grid point using door terrain/creature space, creates the sprite,
fires projectile 144 and attaches opcode 68. Removal uses amount 1,
duration type 4, unsigned (expiry-worldTime)/15, source -1 and the original full
caster level at +0xC4. Duration type 1 skips attachment; the original unused
removal-effect leak is retained.

CMessageDoorOpen uses pack(2), BYTE m_bOpenDoor (BG2 name), three getters and
marshal/unmarshal (0x4FCF00/0x4FD0E0). Its 17-byte packet holds target/source
remote pairs, then the raw open byte. Missing source decodes to INVALID_INDEX
with success; missing target fails, preserving other fields. Missing marshal
objects emit zero/-1 pairs.

The factory's 0x90 case now builds the SPBOOM travelling leaf instead of the
generic projectile with empty Fire. Constructor 0x530300 loads SPBOOM, palette
range 0x43, one direction, no tint/height offset/mirror/shadow or fire sound.
The case then applies range 0x41 and clears both sound refs. AIUpdate 0x5306F0
advances the cell, removes/deletes at sequence end and updates sound coordinates.
Time Stop compares the projectile ID, not source ID.
Fire/Render inherit CProjectileTravelling, per vtable 0x84DFA4.
CProjectileSPBOOM is a BAM-based guessed name; no BG2 match.

## Evidence and validation

Full bundles, callsets, headers and assembly through returns were read.
Case 0x90: jump entry 0x5290B8 -> 0x52326E; palette helper 0x52B150.
Assembly fixes Ghidra's receivers, sprite temporaries and widths.
Bundle SHA-256:

- door/transport: e913fe0a07d9a4441bf70cb2023f7f0252bb36cde65610f6c19313d9d2275e9a
- factory: 004097a29fc95712cee19f41e16d96405e0f90f4d0df252ef79fde952055d37d
- leaf: cc988bcd9fb106337497187ee9de6bd081cb558fbb53c8b0bb258b5d5733427b

The profile centers save 37, tests effects/packets and invokes Greater
Zombie Lord (00SZMBL2.CRE, strref 35642). It checks removal/FireProjectile payloads,
Time Stop and visual retirement, then repeats door, movement, targeting and
melee/bow/crossbow/sling/projectile regressions. Temporary objects, messages,
door fields and remote mappings are cleaned before ticks; isolated checks
never advance the timer or save.

VS2019/v142 Win32 clean build passed. Batch `2026-10-10T09-45-53-315333-batch`
passed 39 steps/3857 assertions (1061 new), six symbol gates, CLEAN crash guard
and profile cleanup. Hits: AddEffect 467, marshal/unmarshal 12 each, factory 22,
leaf ctor 2, AIUpdate 18. Melee dealt 6; bow/crossbow/sling 12 each, one ammo
charge. Leaf parity is GREEN; other four targets YELLOW on call counts. The
factory export's two entry calls do not cover its fragmented switch.

Opening remains incomplete: DoorOpen::Run (0x4FD1D0) is inherited; ToggleDoor
(0x489680) and the network factory entry are absent. Door class audit is clean;
the new message/leaf classes are not parsed by automatic audits. Optional offset
and layout checks skip unavailable analyses; base layouts retain reviewed gaps.
Alternate-selector auto-pause and visual parity remain unproven. No complete
door/factory behavior or differential equivalence is claimed.

Next: ToggleDoor/Run, occupants (0x47A1C0), sounds, key consumption (0x463B30)
and traps. Read full bundles first. Formation executor 0x73EA80 follows.
