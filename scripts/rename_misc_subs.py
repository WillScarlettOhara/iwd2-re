#!/usr/bin/env python3
"""Batch rename CImm IME + CInfGame sub_ functions."""
import re
from pathlib import Path

SRC = Path("/home/wills/projects/IWD2-RE/iwd2-re/src")

replacements = {
    # CImm - IME
    "sub_7C2680": "OnIMEComposition",
    "sub_7C27D0": "OnIMEResult",
    "sub_7C2CC0": "ActivateNativeIME",
    "sub_7C2E10": "DeactivateNativeIME",
    "sub_7C2ED0": "GetIMEStatusText",
    "sub_7C3020": "GetIMECandidateWindowRect",
    "sub_7C3140": "DrawIMECandidateWindow",
    "sub_7C34A0": "GetCandidateList",
    "sub_7C34D0": "GetCompositionString",
    # "sub_7C25B0" → HandleCandidateList (already renamed)
    # CInfGame
    "sub_59FA00": "ReleaseAreaThreadLock",
    "sub_5AF420": "RefreshPortrait",
    # CItem
    "sub_4EA580": "DisplayItemTooltip",
    # CGameSpy
    "sub_4D1EC0": "ConnectToGameSpy",
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
