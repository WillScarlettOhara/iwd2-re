#!/usr/bin/env python3
"""Fix CGameSprite prefix mismatches + rename animation sound flags."""
import re
from pathlib import Path

SRC = Path("/home/wills/projects/IWD2-RE/iwd2-re/src")

# Map: old name → new name (applied everywhere)
replacements = {
    # Animation sound flags: bfield_ in .cpp, m_field_ in .h → m_b
    "bfield_70FB": "m_bHasAttackSound",
    "bfield_70FC": "m_bHasBattleCrySound",
    "bfield_70FD": "m_bHasDamageSound",
    "bfield_70FE": "m_bHasDeathSound",
    "bfield_70FF": "m_bHasFidgetSound",
    "bfield_7100": "m_bHasSelectedSound",
    "bfield_7101": "m_bHasFallSound",
    "m_field_70FB": "m_bHasAttackSound",
    "m_field_70FC": "m_bHasBattleCrySound",
    "m_field_70FD": "m_bHasDamageSound",
    "m_field_70FE": "m_bHasDeathSound",
    "m_field_70FF": "m_bHasFidgetSound",
    "m_field_7100": "m_bHasSelectedSound",
    "m_field_7101": "m_bHasFallSound",
    # Dual class flag: nfield_ in .cpp, m_field_ in .h
    "nfield_70F2": "m_nDualClassFlag",
    "m_field_70F2": "m_nDualClassFlag",
    # Sprite state flags: bfield_ in .cpp, m_field_ in .h
    "bfield_9D14": "m_bSpriteFlag0",
    "bfield_9D15": "m_bSpriteFlag1",
    "m_field_9D14": "m_bSpriteFlag0",
    "m_field_9D15": "m_bSpriteFlag1",
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
