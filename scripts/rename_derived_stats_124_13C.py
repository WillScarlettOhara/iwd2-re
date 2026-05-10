#!/usr/bin/env python3
"""Batch rename CDerivedStats fields 0x124-0x13C."""
import re
from pathlib import Path

SRC = Path("/home/wills/projects/IWD2-RE/iwd2-re/src")

# File-specific replacements
replacements = {
    "CDerivedStats.h": [
        ("nm_field_124", "m_nNaturalArmorBonus"),
        ("nm_field_128", "m_nSTRBonus"),
        ("nm_field_12C", "m_nDEXBonus"),
        ("nm_field_130", "m_nCONBonus"),
        ("nm_field_134", "m_nINTBonus"),
        ("nm_field_138", "m_nWISBonus"),
        ("nm_field_13C", "m_nCHRBonus"),
    ],
    "CDerivedStats.cpp": [
        ("nm_field_124", "m_nNaturalArmorBonus"),
        ("nm_field_128", "m_nSTRBonus"),
        ("nm_field_12C", "m_nDEXBonus"),
        ("nm_field_130", "m_nCONBonus"),
        ("nm_field_134", "m_nINTBonus"),
        ("nm_field_138", "m_nWISBonus"),
        ("nm_field_13C", "m_nCHRBonus"),
    ],
    "CGameEffect.cpp": [
        ("nm_field_128", "m_nSTRBonus"),
        ("bm_field_12C", "m_nDEXBonus"),   # prefix mismatch fix
        ("sm_field_130", "m_nCONBonus"),   # prefix mismatch fix
        ("nm_field_134", "m_nINTBonus"),
        ("nm_field_138", "m_nWISBonus"),
    ],
    "IcewindCGameEffects.cpp": [
        ("m_field_124", "m_nNaturalArmorBonus"),
    ],
}

for fname, reps in replacements.items():
    fpath = SRC / fname
    text = fpath.read_text()
    for old, new in reps:
        # Use word boundary to avoid partial matches
        text = re.sub(rf'\b{re.escape(old)}\b', new, text)
    fpath.write_text(text)
    print(f"Updated {fname}")

print("Done.")
