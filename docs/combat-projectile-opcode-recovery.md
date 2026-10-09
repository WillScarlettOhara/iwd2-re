# Projectile effect (opcode 430)

This lot restores IcewindCGameEffectProjectile::ApplyEffect at 0x56A730.
Constructor 0x4A1C30 installs vtable 0x84C038; slot +0x08 points to this
function. The override was absent, so decoded effects inherited the base
implementation instead of launching their spell payload.

The missing Ghidra function was created through a temporary source address
anchor, reagent_address_map and a complete create-functions/re-export. The
anchor was replaced before building. tmp_combat_opcode_projectile_ctx.md
contains the full recovery context for ApplyEffect and instant Fire. Its
decompiles, required calls, both complete headers and BG2 naming block were
read. Sequential PE instructions from 0x56A730 through ret at 0x56A904 confirm
the branches, operands, ownership and full-width assignments. The decompile
misattributes the pushed level 1 to GetResRef and omits Fire's trailing args;
the assembly establishes the actual calls.

ApplyEffect attempts one ASYNC GetShare of m_sourceID. A failure finishes
successfully with m_done TRUE and no projectile. A successful acquisition
decodes the low WORD of m_dwFlags + 1, passing the recipient sprite to the
factory and cast delay zero. BuildSubSpellEffects receives the acquired source,
m_res and level 1, regardless of the outer effect's packed caster level.
Every returned effect receives sourceRes = m_res, the outer target point and
the full DWORD casterLevel, then is handed to AddEffect. Fire receives the
recipient's area, acquired source ID, recipient ID and actual recipient position,
with height and type zero. The share is released and m_done becomes TRUE.
No source-type or null-projectile guard is added: neither exists in the PE.

The local std::list owns only nodes; the projectile owns the effects. Instant
Fire records IDs/its own area, calls OnArrival and deletes the unregistered
projectile. Travelling Fire registers the missile and ordinary engine ticks
deliver its effects through CMessage105 and CMessageAddEffect. Both payload
target types in ZRCHIT are delivered to the projectile's recipient; this path
does not use ApplyOnHitSpells' self-versus-target routing.

The combat-projectile-opcode profile pins two typed ability tables temporarily:
level-one effects have amounts 1/2, level-two effects 11/12. It exercises decoded
virtual ApplyEffect and Copy, levels 0/1/255/256/257/0x112FF, ordered grouped
messages, source/resource/point metadata, empty payload, missing/stale source,
and arrow/bolt/sling concrete live objects before their first tick. Flags
0x10001 must wrap to the arrow factory case while retaining projectileType 1.
Inspection missiles are removed through RemoveSelf; resources and pending
messages are restored in the same tick. These checks do not run their damage.

The scenario then repeats all previous combat regressions and separately
launches four opcode projectiles (flags 0/1/11/16). Their normal projectile and
message ticks must cause three target HP loss and zero source HP loss each.
The existing deterministic melee and bow/crossbow/sling Swing tests retain
their real HP and ammunition checks. Save/config/fixture setup and cleanup
remain transactional, and the disposable process never saves.

Final batch 2026-10-09T15-48-06-988230-batch passes with five warning targets:
853 checks, 33 scenario steps, all ten PDB gates and a 60-second CLEAN hold.
ApplyEffect executes 17 times, constructor six, Copy once, AddEffect 37,
instant Fire eight, travelling Fire 24, travelling AIUpdate 30, DeliverEffects
26, grouped Run ten and AddEffect Run 46. All 95 isolated projectile checks
pass. The four new launches each cause exactly three recipient HP loss and
zero source loss (recipient 240 -> 237 -> 234 -> 231 -> 228). Existing melee,
riposte and three ranged Swing results retain their expected HP/ammo changes.
Cleanup restores all preserved hashes and removes the fixtures without errors.

A separate clean rebuild compiled every translation unit after the header
change; the batch's build then compiled the final AutoUI additions. ApplyEffect,
Copy, instant Fire, travelling Fire and DeliverEffects have GREEN parity.
Constructor, AddEffect and grouped Run have short-body YELLOW advisories;
travelling AIUpdate and AddEffect Run have call-count YELLOW advisories. The
AIUpdate operand-count advisory and incomplete offset/layout checks remain
unproven. The compiler reports the intentional DWORD-to-USHORT conversion at
the factory boundary; the wrapping case is covered in the typed checks.
Static parity and this clean reconstructed run do not establish matched
original/recovered runtime equivalence, visual parity or multiplayer reception.

Next combat investigation: the missing sprite target-permission helper
0x764810 and its invisibility gates, called from action and status-trigger
dispatch. Establish its real signature and name before recovering it.
