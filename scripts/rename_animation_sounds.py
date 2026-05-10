#!/usr/bin/env python3
import re
from pathlib import Path

SRC = Path("/home/wills/projects/IWD2-RE/iwd2-re/src")

replacements = {
    "sub_7204C0": "LoadAnimationSounds",
    "sub_720B50": "LoadAnimationSoundEntry",
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
