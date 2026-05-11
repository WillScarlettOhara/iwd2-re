#!/usr/bin/env python3
"""Batch rename IcewindMisc + CVisibility + CScreenCharacter high-confidence subs."""
import re
from pathlib import Path

SRC = Path("/home/wills/projects/IWD2-RE/iwd2-re/src")

replacements = {
    # IcewindMisc
    "sub_584610": "DirectionToVector",
    "sub_5847B0": "PointOnCircle",
    "sub_585210": "IsPlayableRace",
    "sub_585310": "IsGoodAlignment",
    "sub_585380": "CreateDamageEffectMagic",
    "sub_5853B0": "CreateDamageEffectFire",
    "sub_5853E0": "CreateDamageEffectCold",
    "sub_585410": "CreateDamageEffectCrushing",
    "sub_585D90": "GetMaxSneakAttackDice",
    "sub_585DA0": "CanSneakAttack",
    "sub_5860F0": "ApplyDamageMod",
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
