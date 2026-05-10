#!/usr/bin/env python3
"""Batch rename CGameSprite sub_ functions (round 2)."""
import re
from pathlib import Path

SRC = Path("/home/wills/projects/IWD2-RE/iwd2-re/src")

replacements = {
    # Combat / THAC0
    "sub_73C6A0": "GetTargetACModForDamageType",
    "sub_73C7E0": "GetRacialLightTHAC0Mod",
    "sub_73C8C0": "GetRacialTHAC0ModVsTarget",
    "sub_73CA20": "GetRacialWeaponTHAC0Mod",
    "sub_73CB10": "GetAbilityAttackMod",
    "sub_73CC40": "GetWeaponTHAC0Bonus",
    "sub_73D420": "GetBaseTHAC0",
    "sub_73D440": "GetWeaponProficiencyTHAC0Mod",
    # Armor
    "sub_724010": "GetMaxDexBonusForArmor",
    "sub_7240A0": "GetArmorCheckPenalty",
    "sub_724170": "GetShieldCheckPenalty",
    "sub_724690": "IsArmorType",
    # Spells
    "sub_724FD0": "AddClassSpell",
    "sub_725110": "AddDomainSpell",
    "sub_725210": "AddInnateSpell",
    "sub_725270": "RemoveInnateSpell",
    "sub_725330": "HasSpell",
    "sub_7256B0": "HasDomainSpell",
    "sub_725840": "HasInnateSpell",
    # Feats / Character
    "sub_763200": "MeetFeatRequirements",
    "sub_763A40": "CanUpgradeFeat",
    "sub_726570": "ApplyFeatEffects",
    "sub_737910": "CheckWeaponAmmunition",
    # Actions / UI
    "sub_71A0E0": "UseSpellFromButton",
    "sub_71A550": "UseItemFromButton",
    "sub_75F240": "SavePositionToBaseStats",
    "sub_7615F0": "SetVisibilityRange",
    "sub_761990": "RefreshEffects",
    "sub_5940E0": "GetCustomButtonValue",
    "sub_594120": "SetCustomButtonValue",
    # State / Misc
    "sub_6FB440": "IsValidActionState",
    "sub_724900": "IsSpellcaster",
    "sub_724920": "IsBard",
    "sub_724C40": "AdjustShapeshiftLevel",
}

for fpath in SRC.glob("*.cpp"):
    text = fpath.read_text()
    changed = False
    for old, new in replacements.items():
        new_text, count = re.subn(rf'\b{re.escape(old)}\b', new, text)
        if count:
            text = new_text
            changed = True
    if changed:
        fpath.write_text(text)
        print(f"Updated {fpath.name}")

for fpath in SRC.glob("*.h"):
    text = fpath.read_text()
    changed = False
    for old, new in replacements.items():
        new_text, count = re.subn(rf'\b{re.escape(old)}\b', new, text)
        if count:
            text = new_text
            changed = True
    if changed:
        fpath.write_text(text)
        print(f"Updated {fpath.name}")

print("Done.")
