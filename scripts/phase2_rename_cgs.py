#!/usr/bin/env python3
"""
Phase 2: Apply confident renames to CGameSprite sub_ functions in Ghidra.
Generates a rename mapping for source code.
"""
import json, subprocess, sys, time

TOKEN = "iwd2ee-re-tools-2026"
BASE = "http://127.0.0.1:8089"

def gh_post(path, data):
    r = subprocess.run(["curl", "-sf", "-X", "POST", "-m", "30",
        "-H", f"Authorization: Bearer {TOKEN}",
        "-H", "Content-Type: application/json",
        "-d", json.dumps(data), f"{BASE}{path}"],
        capture_output=True, text=True)
    return r.stdout.strip()

# CGameSprite renames — CONFIDENT only
# Format: (address_hex, old_sub_name, new_name, rationale)
CGS_RENAMES = [
    # --- Setters/Getters ---
    ("0x00453160", "sub_453160", "CGameSprite::SetCharacterId", "Sets field at 0x7240"),
    ("0x00453170", "sub_453170", "CGameSprite::GetCharacterId", "Gets field at 0x7240"),
    ("0x004531B0", "sub_4531B0", "CGameSprite::SetResurrectionFlag", "Sets *(this+0x562c)=1"),
    ("0x00457100", "sub_457100", "CGameSprite::GetTypeFlags", "Gets field at 0x70F2 (was Getfield_70F2)"),
    ("0x005940D0", "sub_5940D0", "CGameSprite::GetSkillValue", "Gets field at 0x80C (was Getfield_80C)"),
    ("0x005940E0", "sub_5940E0", "CGameSprite::GetButtonData", "Indexed button data getter at 0x3D14"),
    ("0x00594120", "sub_594120", "CGameSprite::SetButtonData", "Indexed button data setter at 0x3D14"),
    
    # --- Spell Memorization ---
    ("0x00724FD0", "sub_724FD0", "CGameSprite::IsSpellMemorizedForClass", "Checks spell in class-specific memory list"),
    ("0x00725110", "sub_725110", "CGameSprite::IsSpellMemorized", "Checks spell in global memory list"),
    ("0x00725270", "sub_725270", "CGameSprite::CanLearnSpell", "Checks if spell can be added to memory"),
    ("0x00725210", "sub_725210", "CGameSprite::TryLearnSpell", "Wraps spell learning logic"),
    ("0x00725330", "sub_725330", "CGameSprite::HasSpellInMemory", "Checks if spell exists in memory"),
    ("0x007256B0", "sub_7256B0", "CGameSprite::MemorizeSpell", "Adds spell to memorized slots"),
    ("0x00725840", "sub_725840", "CGameSprite::ForgetSpell", "Removes spell from memorized slots"),
    ("0x00724C40", "sub_724C40", "CGameSprite::AdjustMemorizedSpellTotal", "Adjusts total memorized count"),
    
    # --- Feat System ---
    ("0x00763150", "sub_763150", "CGameSprite::HasFeat", "Checks if sprite possesses a feat (bitmask at 0x75C)"),
    ("0x00763200", "sub_763200", "CGameSprite::CanUseFeat", "Checks feat usability with stat context"),
    ("0x00763A40", "sub_763A40", "CGameSprite::CanGainFeat", "Validates if feat can be acquired"),
    ("0x00726270", "sub_726270", "CGameSprite::GetFeatValue", "Gets feat-level value (armor prof, expertise, etc.)"),
    ("0x00726330", "sub_726330", "CGameSprite::SetFeatValue", "Sets feat-level value with bounds check"),
    ("0x00726570", "sub_726570", "CGameSprite::ApplyFeatBonuses", "Processes derived feat bonuses"),
    ("0x00726800", "sub_726800", "CGameSprite::GetFeatIconColumn", "Returns feat UI column offset"),
    ("0x00726810", "sub_726810", "CGameSprite::RedrawFeatSlots", "Refreshes feat UI display"),
    
    # --- Combat/Weapon ---
    ("0x0073C6A0", "sub_73C6A0", "CGameSprite::GetAttackRollBonus", "Attack bonus vs target by attack type"),
    ("0x0073C7E0", "sub_73C7E0", "CGameSprite::GetDualWieldPenalty", "Dual-wielding penalty calculation"),
    ("0x0073C8C0", "sub_73C8C0", "CGameSprite::GetCreatureTypeDefense", "Damage reduction by creature type"),
    ("0x0073CA20", "sub_73CA20", "CGameSprite::GetWeaponStyleBonus", "Weapon style-specific attack bonus"),
    ("0x0073CB10", "sub_73CB10", "CGameSprite::GetBaseAttackRoll", "Base attack roll calculation"),
    ("0x0073CC40", "sub_73CC40", "CGameSprite::GetDamageRollBonus", "Damage roll bonus calculation"),
    ("0x0073D420", "sub_73D420", "CGameSprite::GetSpeedFactor", "Speed factor with haste/slow adjustment"),
    ("0x0073D440", "sub_73D440", "CGameSprite::GetAttackModeBonus", "Attack mode-dependent bonus"),
    
    # --- Character State ---
    ("0x007202E0", "sub_7202E0", "CGameSprite::CanAct", "Checks if sprite can perform actions"),
    ("0x007204C0", "sub_7204C0", "CGameSprite::UpdateActionState", "Updates combat/action state machine"),
    ("0x00720B50", "sub_720B50", "CGameSprite::ParseCharacterScript", "Parses INI/script character config"),
    ("0x00723BF0", "sub_723BF0", "CGameSprite::SetInvisible", "Toggles invisibility and updates global counter"),
    ("0x00723F60", "sub_723F60", "CGameSprite::HasResurrectionPenalty", "Checks resurrection penalty flags"),
    ("0x0072DE60", "sub_72DE60", "CGameSprite::ProcessTurn", "Processes one game turn for this creature"),
    ("0x00737910", "sub_737910", "CGameSprite::InterruptSpellCast", "Interrupts spell casting if hit"),
    ("0x00756930", "sub_756930", "CGameSprite::EquipItem", "Equips/uses item (weapon/launcher)"),
    ("0x0075F240", "sub_75F240", "CGameSprite::SnapshotCreatureState", "Saves creature stat snapshot"),
    ("0x0075F3D0", "sub_75F3D0", "CGameSprite::ProcessDeath", "Death/combat-end processing"),
    ("0x007615F0", "sub_7615F0", "CGameSprite::SetMovementRate", "Sets movement speed (walk/run)"),
    ("0x00761650", "sub_761650", "CGameSprite::LoadCreatureHitPoints", "Loads HP/creature data"),
    ("0x00761990", "sub_761990", "CGameSprite::ResetCombatState", "Resets combat tracking globals"),
    
    # --- Saving Throws ---
    ("0x00724010", "sub_724010", "CGameSprite::GetSavingThrow", "Saving throw for given category"),
    ("0x007240A0", "sub_7240A0", "CGameSprite::GetSavingThrowFortitude", "Fortitude save calculation"),
    ("0x00724170", "sub_724170", "CGameSprite::GetSavingThrowReflex", "Reflex save calculation"),
    ("0x00724270", "sub_724270", "CGameSprite::GetSavingThrowWill", "Will save calculation"),
    ("0x00724360", "sub_724360", "CGameSprite::GetSaveBonusByLevel", "Level-based save bonus"),
    
    # --- Status/Checks ---
    ("0x00724430", "sub_724430", "CGameSprite::GetTotalArmorClass", "Total AC with modifiers"),
    ("0x007245D0", "sub_7245D0", "CGameSprite::IsInvisibleOrSanctuary", "Invisibility/sanctuary check"),
    ("0x00724690", "sub_724690", "CGameSprite::CanTargetCreature", "Checks if target can be attacked"),
    ("0x00724900", "sub_724900", "CGameSprite::IsActionAvailable", "Action availability check"),
    ("0x00724920", "sub_724920", "CGameSprite::GetActionStateMask", "Action bitmask check"),
    ("0x006FB440", "sub_6FB440", "CGameSprite::ShouldPlayAnimation", "Determines if animation should play"),
    ("0x0071A0E0", "sub_71A0E0", "CGameSprite::DispatchButtonPress", "Dispatches button command"),
    ("0x0071A550", "sub_71A550", "CGameSprite::DispatchButtonRelease", "Dispatches button release"),
    ("0x0071E760", "sub_71E760", "CGameSprite::RecalculateDerivedStats", "Recalculates derived stats"),
    ("0x0071F6E0", "sub_71F6E0", "CGameSprite::GetCreatureSizeBonus", "Size category-based bonus"),
    ("0x0073CAD0", "sub_73CAD0", "CGameSprite::GetUnusedCombatValue", "Placeholder returning 0"),
    ("0x0073CAE0", "sub_73CAE0", "CGameSprite::ReportCombatError", "Error reporting wrapper"),
]

print(f"Applying {len(CGS_RENAMES)} renames to Ghidra...")
print()

success = 0
failed = 0
mapping = {}  # old_name -> new_name for source code

for i, (addr_hex, old_name, new_name, rationale) in enumerate(CGS_RENAMES):
    sys.stdout.write(f"\r  [{i+1}/{len(CGS_RENAMES)}] {old_name} -> {new_name.split('::')[-1]}  ")
    sys.stdout.flush()
    
    resp = gh_post("/rename_function_by_address", {
        "function_address": addr_hex,
        "new_name": new_name
    })
    
    try:
        data = json.loads(resp)
        if data.get("success") or data.get("result") == "ok" or "renamed" in resp.lower():
            success += 1
            mapping[old_name] = new_name
        else:
            failed += 1
            print(f"\n  FAILED {addr_hex}: {resp}")
    except:
        failed += 1
        print(f"\n  PARSE ERROR {addr_hex}: {resp[:100]}")
    
    time.sleep(0.15)

print(f"\n\nDone! Applied: {success}, Failed: {failed}")
print(f"Total: {success + failed}")

# Save mapping for source code updates
with open('/tmp/cgs_rename_mapping.json', 'w') as f:
    json.dump({
        'success_count': success,
        'failed_count': failed,
        'mapping': mapping,
        'renames': CGS_RENAMES
    }, f, indent=2)

print(f"\nSource code mapping saved to /tmp/cgs_rename_mapping.json")
print(f"\n--- Renames that succeeded ---")
for old, new in mapping.items():
    if '::' in new:
        cls, method = new.split('::', 1)
        print(f"  {old} -> {cls}::{method}")

# Print summary
print(f"\n--- Categories ---")
cats = {}
for _, old_name, new_name, rationale in CGS_RENAMES:
    if 'feat' in rationale.lower() or 'Feat' in new_name:
        cat = "Feats"
    elif 'spell' in rationale.lower() or 'Memoriz' in new_name or 'Spell' in new_name:
        cat = "Spells"
    elif 'attack' in rationale.lower() or 'weapon' in rationale.lower() or 'Weapon' in new_name or 'Attack' in new_name or 'Damage' in new_name or 'Dual' in new_name or 'Combat' in new_name or 'Speed' in new_name:
        cat = "Combat"
    elif 'save' in rationale.lower() or 'Saving' in new_name:
        cat = "Saving Throws"
    elif 'Setter' in rationale or 'Getter' in rationale:
        cat = "Getters/Setters"
    elif 'character' in rationale.lower() or 'Creature' in new_name or 'Invisible' in new_name or 'Death' in new_name:
        cat = "Character State"
    elif 'animation' in rationale.lower() or 'Animation' in new_name or 'Button' in new_name:
        cat = "UI/Input"
    else:
        cat = "Other"
    cats[cat] = cats.get(cat, 0) + 1

for cat, count in sorted(cats.items()):
    print(f"  {cat}: {count}")
