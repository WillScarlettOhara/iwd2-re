#!/usr/bin/env python3
"""Rename final implemented non-stub subs."""
import re
from pathlib import Path

SRC = Path("/home/wills/projects/IWD2-RE/iwd2-re/src")

replacements = {
    "sub_43E300": "RenderBinkFrame",
    "sub_63E150": "ShowLoadError",
    "sub_63E440": "DismissLoadError",
    "sub_66A010": "GetSpellLevelCount",
    "sub_6734F0": "UpdateItemDescription",
    "sub_673740": "UpdateSpellDescription",
    "sub_642C90": "IsPointOnSprite",
}

for f in SRC.glob("*.cpp"):
    t = f.read_text(); ch = False
    for o,n in replacements.items():
        t2,c = re.subn(rf'\b{re.escape(o)}\b', n, t)
        if c: t = t2; ch = True
    if ch: f.write_text(t); print(f.name)

for f in SRC.glob("*.h"):
    t = f.read_text(); ch = False
    for o,n in replacements.items():
        t2,c = re.subn(rf'\b{re.escape(o)}\b', n, t)
        if c: t = t2; ch = True
    if ch: f.write_text(t); print(f.name)

print("Done.")
