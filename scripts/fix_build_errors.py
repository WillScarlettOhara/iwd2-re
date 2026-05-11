#!/usr/bin/env python3
"""Fix all remaining field name mismatches between .h and .cpp that cause build errors."""
import re
from pathlib import Path

SRC = Path("/home/wills/projects/IWD2-RE/iwd2-re/src")

# Each entry: (old_cpp_name, new_correct_name_from_header)
# These are cases where .cpp uses wrong prefix/name vs .h declaration
replacements = {
    # CGameSprite.cpp uses nfield_/bfield_/wfield_ but header has m_field_
    "nfield_54A8": "m_field_54A8",
    "bfield_7430": "m_field_7430",
    "nfield_562C": "m_field_562C",
    "nfield_5640": "m_field_5640",
    "bfield_725A": "m_field_725A",
    "nfield_5304": "m_field_5304",
    "nfield_536A": "m_field_536A",
    "nfield_536E": "m_field_536E",
    "nfield_53D2": "m_field_53D2",
    "nfield_53E6": "m_field_53E6",
    "nfield_54B8": "m_field_54B8",
    "nfield_557A": "m_field_557A",
    "nfield_557E": "m_field_557E",
    "nfield_5582": "m_field_5582",
    "nfield_558E": "m_field_558E",
    "wfield_559E": "m_field_559E",
    "wfield_55A0": "m_field_55A0",
    "nfield_55A2": "m_field_55A2",
    "nfield_55F2": "m_field_55F2",
    "nfield_55F6": "m_field_55F6",
    "nfield_55FA": "m_field_55FA",
    "nfield_55FE": "m_field_55FE",
    "bfield_5602": "m_field_5602",
    "wfield_560C": "m_field_560C",
    "wfield_560E": "m_field_560E",
    "wfield_5610": "m_field_5610",
    "wfield_5612": "m_field_5612",
    "wfield_5614": "m_field_5614",
    "wfield_5616": "m_field_5616",
    "nfield_5618": "m_field_5618",
    "wfield_561C": "m_field_561C",
    "wfield_561E": "m_field_561E",
    "bfield_5630": "m_field_5630",
    "nfield_5632": "m_field_5632",
    "bfield_5636": "m_field_5636",
    "nfield_70F6": "m_field_70F6",
    "nfield_70F7": "m_field_70F7",
    "nfield_70F8": "m_field_70F8",
    "nfield_70F9": "m_field_70F9",
    "nfield_70FA": "m_field_70FA",
    "nfield_7106": "m_field_7106",
    "wfield_710A": "m_field_710A",
    "wfield_710C": "m_field_710C",
    "wfield_710E": "m_field_710E",
    "nfield_7110": "m_field_7110",
    "nfield_7118": "m_field_7118",
    "wfield_711C": "m_field_711C",
    "nfield_72D6": "m_field_72D6",
    "nfield_72DE": "m_field_72DE",
    "nfield_72E2": "m_field_72E2",
    "nfield_7532": "m_field_7532",
    "nfield_7536": "m_field_7536",
    "nfield_753A": "m_field_753A",
    "nfield_753C": "m_field_753C",
    "nfield_7540": "m_field_7540",
    "nfield_7544": "m_field_7544",
    "pfield_7548": "m_field_7548",
    "pfield_9088": "m_field_9088",
    "nfield_9D08": "m_field_9D08",
    "nfield_9D0C": "m_field_9D0C",
    "nfield_9D10": "m_field_9D10",
    "bfield_9D14": "m_field_9D14",
    "bfield_9D15": "m_field_9D15",
    "bfield_72A8": "m_field_72A8",
    "nfield_72AA": "m_field_72AA",
    "wfield_72A0": "m_field_72A0",
    "wfield_72A2": "m_field_72A2",
    "nm_field_4C54": "m_m_field_4C54",
    "nfield_70EE": "m_field_70EE",
    "nfield_70F6": "m_field_70F6",
    # IcewindCGameEffects.cpp uses field_FOO (no prefix) for CGameSprite members
    "field_9D15": "m_field_9D15",
    "field_C": "m_nACDeflectionBonus",
    # CAIGroup.cpp / CSearchBitmap.cpp / CMessage.cpp use old CGameSprite names
    "bfield_7430": "m_field_7430",
    "nfield_54A8": "m_field_54A8",
    "nfield_562C": "m_field_562C",
    # CGameEffect.cpp uses old names
    "nfield_C": "m_nACDeflectionBonus",
    "wfield_6": "m_nArmorClass",
    "wfield_A": "m_nACDodgeBonus",
    "wm_field_48": "m_nDurationMod",
    # CGameEffectList.cpp / other files
    "bfield_9D15": "m_field_9D15",
    "bfield_9D14": "m_field_9D14",
    # CSoundMixer
    "nfield_C0": "m_nSoundFlags",
    # CUIControlTextDisplay
    "bfield_5A": "m_bField5A",
    "nfield_5C": "m_nField5C",
    # CChitin / CBaldurChitin fields already renamed in header
    "nm_field_1A4": "m_bIMEEnabled",
    "nm_field_19C": "m_nAISleeper",
    "wfield_FE": "m_nCapsLockState",
    "nm_field_110": "m_bStartUpHost",
    "bm_field_131": "m_bStartUpGameSpyLocation",
    "pm_m_field_106": "m_m_field_106",
    "nm_m_field_142": "m_m_field_142",
    "nm_field_13E": "m_nField13E",
    "wm_field_4C": "m_dwField4C",
    "nfield_C4": "m_bDirectSoundInit",
    "m_field_E8": "m_rClient",
    "sm_field_130": "m_sField130",
    "nm_field_20": "m_nField20",
    # CGameSpy
    "bm_field_0": "m_bWinsockInitialized",
    "bm_field_1": "m_bConnected",
    # CGameArea
    "pm_field_1FC": "m_critSect1FC",
    "pm_field_214": "m_critSect214",
    "wm_field_4C": "m_dwHeaderField4C",
    "bm_field_241": "m_bScrollTimer",
    "wm_field_3EC": "m_nField3EC",
    "bm_field_436": "m_bField436",
    "nm_field_438": "m_nField438",
    "bfield_AE6": "m_bFieldAE6",
    "nfield_B0E": "m_nFieldB0E",
    "bfield_B16": "m_bFieldB16",
    # CDimm
    "wfield_D2": "m_wFieldD2",
    "bfield_E6": "m_bFieldE6",
    "pfield_EA": "m_pFieldEA",
    # IcewindMisc / CGameEffect
    "m_field_10": "m_field_10",
    # CGameOptions
    "bfield_AC": "m_bFieldAC",
    # CGameAnimationTypeEffect
    "wfield_5DE": "m_wField5DE",
    # IcewindCVisualEffect
    "m_field_0": "bm_field_0",
    "m_field_1": "bm_field_1",
    "m_field_2": "bm_field_2",
    "m_field_3": "bm_field_3",
}

for f in SRC.glob("*.cpp"):
    t = f.read_text(); ch = False
    for o,n in replacements.items():
        t2,c = re.subn(rf'\b{re.escape(o)}\b', n, t)
        if c: t = t2; ch = True
    if ch: f.write_text(t); print(f"FIXED {f.name}")

print("Done.")
