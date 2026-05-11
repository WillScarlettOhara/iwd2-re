#!/usr/bin/env python3
"""Rename CScreenStore sell + CNetwork + CInfGame subs."""
import re
from pathlib import Path

SRC = Path("/home/wills/projects/IWD2-RE/iwd2-re/src")

replacements = {
    "sub_67A130": "SellSelectedItems",
    "sub_67A5B0": "SellSelectedBagItems",
    "sub_67ABF0": "SellSelectedItemsToStore4",
    "sub_67B030": "SellSelectedBagItemsToStore4",
    "sub_7A61D0": "IsPasswordRequired",
    "sub_5A0160": "InitializeGameAreas",
    "sub_5BDBA0": "SetAreaGlobalFlag",
    "sub_5C3770": "GetDefaultBiography",
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
