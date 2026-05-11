#!/usr/bin/env python3
"""Batch rename CGameSpriteSpells sub_ functions."""
import re
from pathlib import Path

SRC = Path("/home/wills/projects/IWD2-RE/iwd2-re/src")

replacements = {
    "sub_725C50": "AddCasts",
    "sub_725CC0": "RemoveCasts",
    "sub_725D30": "AddAllCasts",
    "sub_725DB0": "RemoveAllCasts",
    "sub_725F00": "CountTotalCasts",
    "sub_726060": "AddCasts",
    "sub_7260B0": "RemoveCasts",
    "sub_726150": "AddAllCasts",
    "sub_7261A0": "RemoveAllCasts",
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
