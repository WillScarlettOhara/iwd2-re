# Ranged UI validation — 2026-09-20

The recovered Attack/Swing chain now has an ordinary UI scenario for bow,
crossbow and sling. This change adds test scaffolding, not a new combat rule.
It does not require the player's characters to own these weapons.

`rangedready` retains the actor's equipment, weapon selection and quick buttons,
then installs one ZRC launcher and 40 matching ammunition. Weapon changes pass
through SelectWeaponAbility/SelectLauncherAmmo. Switching through the retained
fist slot removes the original weapon's equipped effects before replacing its
pointer, and removes fixture equipment before restoring the real weapon.

Each test clicks the normal weapon button and party member 1, requires the
ground picker to identify that member, observes action 3 and actual HP loss,
then stops the attack. `rangedrestore` requires ammunition consumption and
checks the restored item pointers and selected ability. No hit rolls or combat
frames are forced. Damage numbers below are observations, not fixed assertions.

Final profile: `.arc/2026-09-19T23-55-32-376472-batch/`.

| Weapon | Target HP | Damage | Ammunition |
|---|---|---:|---|
| Bow / arrow | 291 -> 283 | 8 | 40 -> 39 |
| Crossbow / bolt | 283 -> 271 | 12 | 40 -> 39 |
| Sling / stone | 271 -> 260 | 11 | 40 -> 39 |

All 48 steps passed. VS2019 Win32 build passed, followed by 120 seconds CLEAN;
Attack and Swing each executed 30 times. The camera was centered and both
members checked visible. Fixture removal and save/config restoration were
hash-verified without cleanup errors. No save was written.

The first run (`.arc/2026-09-19T23-51-17-144876-batch/`) also passed, but used
pointer swapping that skipped unequipping the original weapon. It is retained
as preliminary evidence; the final run uses the corrected equipment lifecycle.

Existing static warnings remain: call-count parity YELLOW for Attack and Swing,
and a Swing string-provenance review warning. No matched original/reconstruction
capture exists. This validates the three basic projectile paths with test items,
not all commercial items, visual weapon assets or special ammunition effects.

Next combat work: test last-ammunition depletion and melee fallback through
Swing/CheckItemDepleted, then compare the same scenario against the original.
