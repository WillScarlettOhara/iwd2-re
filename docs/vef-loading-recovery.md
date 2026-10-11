# VEF loading and projectile attachment

Recovery lot: CResBinary ctor/Demand/Release (4028F0/402940/402980),
CVisualEffectFile ctor/GetVEF (5544C0/5545E0), CVisualEffect::Load (554D20),
VVC/VEF allocation in 423210, and the trailing branches of Fire at
52C050/5791D0. These branches now create and register their controller.

Evidence: full context bundles `tmp_binary_resource_ctx.md`,
`tmp_allocres_ctx.md`, `tmp_vef_named_ctx.md`, `tmp_vef_fire_ctx.md`;
complete reachable disassembly, including CString uppercase and both inlined
message constructors. BG2 names informed naming only; IWD2 offsets and asm
define the layouts and arguments.

Native details preserved:
- CResBinary has a version pointer at +50 and a parsed flag at +54. The ctor
  only initializes the flag. Demand records data+4, nested demand retains the
  pointer, and the last Release clears the flag. The reviewed constructor
  finding is recorded in arc-baseline.txt; no invented pointer initialization.
- Resource types 1019/1020 allocate CResBinary. The VEF helper requests its
  resource and releases/cancels it through the existing helper lifecycle.
- GetVEF copies entire E0-byte records and uppercases each resref. Both file
  arrays append to the timed list at +24A; the triggered list remains empty.
  Only target type, speed and duration are copied into the controller prefix.
  Native signature/version/bounds validation is absent.
- Load uppercases a private CString copy. Only the paired (-1,-1) target
  sentinel falls back to the origin. It registers in LIST_FRONT and seeds Y
  with signed division *before* the final fixed-point shift. The native failed
  Add path reads the ID after deletion; ours captures the unchanged value
  before deletion. Allocation failure is not runtime-injected.
- Both Fire branches run for any nonempty resource, regardless of a previous
  controller ID. They use the original target point, copy signed velocity
  into the controller speed, and OR flags with 1. Message metadata carries
  the projectile ID before registration, target ID, area name, origin, original
  target point and cast height; it does not carry flight distance or type.
- The VFX projectile prefix is now packed to 2 bytes: mirror bytes at +2A0/+2A1
  followed immediately by the visual descriptor at +2A2. Compiler auditing
  exposed a pre-existing two-byte pad there; inline guess comments also hid
  the mirror bytes from the layout parser and were moved after the declarations.

AutoUI `vefloadchecks` uses a unique temporary KEY slot and an in-memory pinned
resource. It verifies demand states, complete record copies, array ordering,
empty/ignored headers, null/missing resources, independent ownership, signed
and wrapped coordinates, both projectile families and queued message fields.
It removes every new object/message and restores the key, demand accounting,
area queues and ASYNC deny counts within the same tick. The combat-messages
profile retains the actual melee/bow/crossbow/sling HP and ammunition checks.

Scope: controller/event loading is recovered; this does not establish complete
event-to-animation creation or remote trailing-message serialization. Existing
Fire behavior outside the trailing branches is not a new binary-parity claim.
Next: identify the remaining animation construction path and recover message
Marshal/Unmarshal at 511D00/511F40 before claiming multiplayer VEF attachment.

Validation: `.arc/2026-10-11T11-50-40-340913-batch`, full nine-target profile:
PASS with five warnings (reviewed ctor initialization and call counts).
Four GREEN/five YELLOW parity results; compiled resource/controller/projectile
spacing checks PASS (2/12/3 members). Offset oracles remain not analysable;
there is no matched original/reconstruction differential capture.
Win32 Debug build PASS; 88 VEF cases and all 51 UI steps PASS; 9/9 symbols hit;
60-second hold CLEAN; profile cleanup PASS. Actual Swing HP loss: melee 6,
arrow/bolt/sling 12 each. No save was written.
