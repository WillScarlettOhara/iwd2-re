#!/usr/bin/env python3
"""Two-way sync: rename .h fields to match .cpp after fix_build_errors.py ran."""
import re
from pathlib import Path

SRC = Path("/home/wills/projects/IWD2-RE/iwd2-re/src")

# Map of .h old field → new name that .cpp now uses
# ONLY for headers that weren't synced by fix_build_errors.py
header_renames = {
    # CGameEffect.h — CPersistantEffect84C4A4 fields
    "CGameEffect.h": {
        "nm_field_18": "m_nStartTime",
        "nm_field_1C": "m_nDeadline",
        "nm_field_20": "m_nField20",
        "nm_field_24": "m_nTickInterval",
        "bm_field_28": "m_bEnabled",
        "sm_field_29": "m_resName",
        "bm_field_31": "m_bField31",
        "bm_field_32": "m_bField32",
        "bm_field_33": "m_bField33",
        # Subclass fields
        "nm_field_34": "m_sField34",
        "nm_field_38": "m_nField38",
        "nm_field_44": "m_sField44",
        "nfield_54": "m_nField54",
        "nfield_58": "m_setField58",
        "nfield_68": "m_nField68",
    },
    # CDimm.h fields
    "CDimm.h": {
        "wfield_D2": "m_wFieldD2",
        "bfield_E6": "m_bFieldE6",
        "pfield_EA": "m_pFieldEA",
    },
    # CGameAnimationTypeEffect.h  
    "CGameAnimationTypeEffect.h": {
        "wfield_5DE": "m_wField5DE",
    },
    # CChitin.h — fields accessed as g_pBaldurChitin->m_pEngineWorld->...
    "CChitin.h": {
        "nfield_C4": "m_bDirectSoundInit",
        "nm_field_13E": "m_nField13E",
        "nm_field_1A4": "m_bIMEEnabled",
        "nm_field_19C": "m_nAISleeper",
    },
    # CBaldurChitin.h
    "CBaldurChitin.h": {
        "field_F9": "m_field_F9",
        "field_1A72": "m_field_1A72",
        "field_1AB2": "m_field_1AB2", 
        "field_1AF2": "m_field_1AF2",
        "field_4A2C": "m_field_4A2C",
        "field_FB0": "m_field_FB0",
    },
    # CBaldurProjector.h
    "CBaldurProjector.h": {
        "field_145": "m_field_145",
        "field_66A": "m_bField66A",
    },
    # CScreenWorld.h — m_m_field_106 etc
    "CScreenWorld.h": {
        "m_m_field_106": "m_critSectEngine",
        "m_m_field_11E": "m_pVidMode",
        "m_m_field_14A": "m_field_14A",
        "m_m_field_14E": "m_field_14E",
        "m_m_field_15C": "m_field_15C",
        "m_m_field_15D": "m_field_15D",
        "m_m_field_15E": "m_field_15E",
        "m_m_field_160": "m_field_160",
        "m_m_field_162": "m_field_162",
        "m_m_field_164": "m_field_164",
    },
    # CVidBlitter.h
    "CVidBlitter.h": {
        "field_E6": "m_bFieldE6",
    },
}

for h_name, renames in header_renames.items():
    h_path = SRC / h_name
    if not h_path.exists(): 
        print(f"MISSING: {h_name}")
        continue
    t = h_path.read_text()
    t2 = t
    for old, new in renames.items():
        t2 = re.sub(rf'\b{re.escape(old)}\b', new, t2)
    if t2 != t:
        h_path.write_text(t2)
        print(f"SYNCED {h_name}")

print("Done.")
