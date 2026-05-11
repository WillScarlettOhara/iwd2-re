#!/usr/bin/env python3
"""Final batch: high-confidence implemented subs only. Stubs/returns-constant skip."""
import re
from pathlib import Path

SRC = Path("/home/wills/projects/IWD2-RE/iwd2-re/src")

replacements = {
    # Code reads: GetPlayerAddress → gets local IP
    "sub_7A73D0": "GetLocalAddress",
    # Code reads: return sm_field_130; sm_field_130 = "";
    "sub_4D89E0": "PopResultString",
    # Code reads: return m_sMultiplayerSaveDir + m_sSaveGame + "\\";
    "sub_5C0B30": "GetSaveGameDir",
    # Code reads: atoi(area.Mid(2)) >= 9000 → returns TRUE
    "sub_5C79C0": "IsExpansionArea",
    # Code reads: iterates characters, checks nfield_70F2 == 1 → count
    "sub_5C93E0": "CountDualClassedCharacters",
    # Code reads: sets bfield_9D15 = 1 on alive characters
    "sub_5CADF0": "WakeAllCharacters",
    # Code reads: return pArea->bfield_B16
    "sub_5AC0A0": "IsAreaSaveDisabled",
    # Code reads: updates bfield_B16 for all loaded areas
    "sub_5AC0D0": "UpdateAreaSaveStatus",
}

for f in SRC.glob("*.cpp"):
    t = f.read_text(); changed = False
    for o,n in replacements.items():
        t2,c = re.subn(rf'\b{re.escape(o)}\b', n, t)
        if c: t = t2; changed = True
    if changed: f.write_text(t); print(f.name)

for f in SRC.glob("*.h"):
    t = f.read_text(); changed = False
    for o,n in replacements.items():
        t2,c = re.subn(rf'\b{re.escape(o)}\b', n, t)
        if c: t = t2; changed = True
    if changed: f.write_text(t); print(f.name)

print("Done.")
