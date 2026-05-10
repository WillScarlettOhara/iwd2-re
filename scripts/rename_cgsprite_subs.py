#!/usr/bin/env python3
"""Batch rename CGameSprite sub_ functions."""
import re
from pathlib import Path

SRC = Path("/home/wills/projects/IWD2-RE/iwd2-re/src")

replacements = {
    "sub_763150": "HasFeat",
    "sub_726270": "GetFeatMode",
    "sub_726330": "SetFeatMode",
    "sub_726800": "GetWeaponSlot",
    "sub_726810": "SetWeaponSet",
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
