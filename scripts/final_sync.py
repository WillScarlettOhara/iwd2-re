#!/usr/bin/env python3
"""Comprehensive two-way sync of ALL field name mismatches between .h and .cpp."""
import re
from pathlib import Path

SRC = Path("/home/wills/projects/IWD2-RE/iwd2-re/src")

# ALL remaining mismatches extracted from build log. Each mapping shows
# old_name -> new_name. Applied to BOTH .h and .cpp files.
ALL_RENAMES = {
    # === CGameEffect.h renamed to new names, but .cpp still uses old → fix .cpp ===
    "nm_field_18": "m_nStartTime",
    "nm_field_1C": "m_nDeadline", 
    "nm_field_20": "m_nField20",
    "nm_field_24": "m_nTickInterval",
    "bm_field_28": "m_bEnabled",
    "sm_field_29": "m_resName",
    "bm_field_31": "m_bField31",
    "bm_field_32": "m_bField32",
    "bm_field_33": "m_bField33",
    "nm_field_34": "m_sField34",
    "nm_field_38": "m_nField38",
    "nm_field_44": "m_sField44",
    "nfield_54": "m_nField54",
    "nfield_58": "m_setField58",
    "nfield_68": "m_nField68",

    # === Headers that still have old names, but .cpp uses new from fix_build_errors.py ===
    # CVidBlitter.h: bfield_E6 → our script renamed .cpp to m_bFieldE6
    "bfield_E6": "m_bFieldE6",
    # CGameAnimationTypeEffect.h: bfield_5DE → our script renamed .cpp to m_wField5DE  
    "bfield_5DE": "m_wField5DE",
    # CDimm.h: nfield_D2 → .cpp uses m_wFieldD2
    "nfield_D2": "m_wFieldD2",
    # CDimm.h: nfield_E6 → .cpp uses m_bFieldE6
    "nfield_E6": "m_bFieldE6",
    # CDimm.h: wfield_EA → .cpp uses m_pFieldEA
    "wfield_EA": "m_pFieldEA",

    # === CScreenWorld.h still has old names, .cpp uses new ===
    "pm_m_field_106": "m_critSectEngine",
    "nm_m_field_14A": "m_field_14A",
    "nm_field_EA4": "m_field_EA4",

    # === CBaldurProjector.h still has old names ===
    "nm_m_field_106": "m_field_106",
    "pm_m_field_14A": "m_font14A",
    "nfield_66A": "m_bField66A",

    # === CChitin.h still has old names ===
    # .cpp uses m_bWinsockInitialized for CGameSpy, not CChitin. The error is:
    # CDimm.cpp references g_pChitin->pm_field_2FC/pm_field_314 (already fixed in prev commit)
    # CChitin.cpp references m_bDirectSoundInit (originally pfield_C4), m_nCapsLockState, m_nField13E, wm_m_field_142
    # CChitin.h still has pfield_C4, wfield_FE, nm_field_13E, nm_m_field_142 — need to match .cpp
    "pfield_C4": "m_critSectC4",
    "wfield_FE": "m_nCapsLockState",
    "nm_field_13E": "m_nField13E",
    "nm_m_field_142": "m_nField142",
    "pm_field_2FC": "m_critSect2FC",
    "pm_field_314": "m_critSect314",
    "pm_field_32C": "m_critSect32C",
    "pm_field_344": "m_critSect344",
    "pm_field_35C": "m_critSect35C",
    "pm_field_394": "m_critSect394",
    "pm_field_1C32": "m_critSect1C32",

    # CGameSprite.cpp refs that were renamed in .cpp but header has matching names already
    # bfield_70F6 → .cpp still uses old, .h has m_field_70F6  
    "bfield_70F6": "m_field_70F6",
    "bfield_70F7": "m_field_70F7",
    "bfield_70F8": "m_field_70F8",
    "bfield_70F9": "m_field_70F9",
    "bfield_70FA": "m_field_70FA",
    "sfield_56E4": "m_sField56E4",
    "m_field_722A": "m_m_field_722A",
    "nfield_727E": "m_field_727E",
    "bfield_7282": "m_field_7282",
    "bfield_7283": "m_field_7283",
    "bfield_7284": "m_field_7284",
    "nfield_728E": "m_field_728E",
    "nfield_740C": "m_field_740C",
    "sfield_7408": "m_sField7408",
    "nfield_7414": "m_field_7414", 
    "sfield_7410": "m_sField7410",
    "pfield_7398": "m_pField7398",
    "sfield_7418": "m_sField7418",
    "nfield_741C": "m_field_741C",
    "pfield_73EC": "m_pField73EC",
    "sfield_7428": "m_sField7428",
    "nfield_742C": "m_field_742C",
    "pfield_73B4": "m_pField73B4",
    "sfield_7420": "m_sField7420",
    "nfield_7424": "m_field_7424",
    "bm_field_3D3A": "m_m_field_3D3A",
    "nm_field_3D14": "m_m_field_3D14",
    "sfield_7421": "m_sField7421",
    "sfield_7429": "m_sField7429",

    # wm_m_field_142 in CChitin.cpp → needs to match CChitin.h
    "wm_m_field_142": "m_nField142",

    # CGameDialog fields
    "pfield_56": "m_pField56",

    # CGameOptions field  
    "bfield_AC": "m_bFieldAC",

    # CUIControlTextDisplay
    "wfield_A6C": "m_wFieldA6C",
    "bfield_5A": "m_bField5A",
    "nfield_5C": "m_nField5C",

    # CScreenSinglePlayer  
    "nm_field_460": "m_nField460",

    # CSoundMixer missing GetSongPlaying — add forward decl
    # CItem wm_field_48 → m_nDurationMod
    "wm_field_48": "m_nDurationMod",

    # CInfinity nfield_58 → field name in CResInfTile
    "nfield_58": "m_nField58",

    # CBaldurChitin sm_field_130, nm_field_110, bm_field_131
    "sm_field_130": "m_sField130",
    "nm_field_110": "m_nField110",
    "bm_field_131": "m_bField131",
    "pm_m_field_106": "m_critSect106",

    # CScreenKeymaps  
    "dword_8B3FE0": "m_dword8B3FE0",
    "nfield_74": "m_nField74",
    "nfield_76": "m_nField76",
    "bfield_61": "m_bField61",

    # IcewindCVisualEffect
    "m_field_0": "m_bVisual0",
    "m_field_1": "m_bVisual1",
    "m_field_2": "m_bVisual2",
    "m_field_3": "m_bVisual3",

    # CScreenWorld m_m_field_ prefixed fields
    "m_m_field_10B2": "m_field_10B2",
    "m_m_field_10B4": "m_field_10B4",
    "m_m_field_10C0": "m_field_10C0",
    "m_m_field_10C4": "m_field_10C4", 
    "m_m_field_10C8": "m_field_10C8",
    "m_m_field_10CC": "m_field_10CC",
    "m_m_field_10D0": "m_field_10D0",
    "m_m_field_10D8": "m_field_10D8",
    "m_m_field_10F0": "m_field_10F0",
    "m_m_field_10F4": "m_field_10F4",
    "m_m_field_1108": "m_field_1108",
    "m_m_field_110C": "m_field_110C",
    "m_m_field_114C": "m_field_114C",
    "m_m_field_1150": "m_field_1150",
    "m_m_field_119D": "m_field_119D",
    "m_m_field_11B6": "m_field_11B6",
    "m_m_field_11BA": "m_field_11BA",
    "m_m_field_11BE": "m_field_11BE",
    "m_m_field_11D0": "m_field_11D0",
    "m_m_field_11F0": "m_field_11F0",
    "m_m_field_11F4": "m_field_11F4",
    "m_m_field_11F8": "m_field_11F8",
    "m_m_field_1204": "m_field_1204",
    "m_field_F37": "m_nFieldF37",
    "m_field_F44": "m_nFieldF44",
    # Undo the bad .h rename from sync_headers.py
    "m_critSectEngine": "m_critSect106",

    # wfield_D2 in CDimm.cpp  
    "wfield_D2": "m_wFieldD2",
    "bfield_E6": "m_bFieldE6",
    "pfield_EA": "m_pFieldEA",
    "bm_field_2FC": "pm_field_2FC",
    "sm_field_314": "pm_field_314",

    # CResCell nm_field_20
    "nm_field_20": "m_nField20",

    # CResInfTile nfield_58
    "nfield_58": "m_nField58",

    # CScreenChapter
    "bfield_5A": "m_bField5A",

    # CScreenMultiPlayer
    "bm_field_8A0": "m_bField8A0",
    "sm_field_868": "m_sField868",

    # IcewindMisc
    "m_field_10": "m_bField10",
    "bm_field_1E": "m_bField1E",

    # CGameContainer / CScreenWorldMap etc
    "bm_field_8D": "m_bField8D",
    "wfield_666": "m_wField666",
    "wfield_668": "m_wField668",
    "pfield_66E": "m_pField66E",
    "bfield_7B0": "m_bField7B0",
    "nfield_676": "m_nField676",
    "nfield_70F2": "m_field_70F2",

    # CScreenConnection
    "field_FB0": "m_field_FB0",

    # main.cpp
    "m_m_field_1932": "m_field_1932",

    # CCreatureFileHeader
    "m_field_2E9": "m_nSpellMemorization",
    "m_field_2EA": "m_nSpellCount",
    "m_field_2F7": "m_nSpellKnown",
}

for f in list(SRC.glob("*.cpp")) + list(SRC.glob("*.h")):
    t = f.read_text()
    t2 = t
    for old, new in ALL_RENAMES.items():
        t2 = re.sub(rf'\b{re.escape(old)}\b', new, t2)
    if t2 != t:
        f.write_text(t2)
        print(f"FIXED {f.name}")

print("Done.")
