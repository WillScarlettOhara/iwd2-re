#!/usr/bin/env python3
"""Fix header/cpp field name prefix mismatches.

For each mismatch, the header declaration is authoritative.
This script renames the cpp references to match the header.
"""

import re
import sys

MISMATCHES = [
    # (cpp_file, old_name_in_cpp, new_name_from_header)
    ("CAlias.cpp", "nm_field_4", "bm_field_4"),
    ("CAlias.cpp", "nm_field_8", "sm_field_8"),
    ("CBaldurProjector.cpp", "wm_field_144", "bm_field_144"),
    ("CButtonData.cpp", "wm_field_10", "nm_field_10"),
    ("CButtonData.cpp", "bm_field_1e", "wm_field_1e"),
    ("CChitin.cpp", "wm_field_4C", "nm_field_4C"),
    ("CChitin.cpp", "wm_field_11C", "sm_field_11C"),
    ("CChitin.cpp", "sm_field_130", "bm_field_130"),
    ("CChitin.cpp", "bm_field_2FC", "pm_field_2FC"),
    ("CChitin.cpp", "sm_field_314", "pm_field_314"),
    ("CDerivedStats.cpp", "nm_field_8", "wm_field_8"),
    ("CDerivedStats.cpp", "bm_field_12C", "nm_field_12C"),
    ("CDerivedStats.cpp", "sm_field_130", "nm_field_130"),
    ("CDimm.cpp", "bm_field_0", "nm_field_0"),
    ("CDimm.cpp", "nm_field_8", "wm_field_8"),
    ("CGameAnimationTypeMonsterMulti.cpp", "nm_field_44A", "pm_field_44A"),
    ("CGameAreaNotes.cpp", "nm_field_8C", "bm_field_8C"),
    ("CGameEffect.cpp", "nm_field_28", "bm_field_28"),
    ("CGameEffect.cpp", "nm_field_32", "bm_field_32"),
    ("CGameEffect.cpp", "bm_field_38", "nm_field_38"),
    ("CGameEffect.cpp", "wm_field_48", "nm_field_48"),
    ("CGameOptions.cpp", "bm_field_38", "nm_field_38"),
    ("CGameSpriteSpells.cpp", "wm_field_10", "nm_field_10"),
    ("CGameSpy.cpp", "bm_field_2", "nm_field_2"),
    ("CGameSpy.cpp", "pm_field_12", "nm_field_12"),
    ("CGameSpy.cpp", "pm_field_16", "nm_field_16"),
    ("CGameSpy.cpp", "bm_field_1A", "nm_field_1A"),
    ("CGameSpy.cpp", "bm_field_1E", "nm_field_1E"),
    ("CGameSpy.cpp", "wm_field_22", "nm_field_22"),
    ("CImm.cpp", "nm_field_134", "sm_field_134"),
    ("CImm.cpp", "nm_field_13C", "pm_field_13C"),
    ("CInfButtonArray.cpp", "bm_field_0", "nm_field_0"),
    ("CInfButtonArray.cpp", "wm_field_10", "nm_field_10"),
    ("CMessage.cpp", "wm_field_10", "bm_field_10"),
    ("CRes.cpp", "nm_field_20", "pm_field_20"),
    ("CResFile.cpp", "nm_field_20", "pm_field_20"),
    ("CScreenCharacter.cpp", "sm_field_866", "nm_field_866"),
    ("CScreenInventory.cpp", "sm_field_114", "nm_field_114"),
    ("CScreenInventory.cpp", "wm_field_11C", "bm_field_11C"),
    ("CScreenJournal.cpp", "nm_field_484", "sm_field_484"),
    ("CScreenStart.cpp", "wm_field_144", "nm_field_144"),
    ("CSoundMixer.cpp", "bm_field_0", "nm_field_0"),
    ("CSoundMixer.cpp", "wm_field_144", "pm_field_144"),
    ("CSoundMixer.cpp", "pm_field_178", "nm_field_178"),
    ("CSpawn.cpp", "nm_field_13E", "bm_field_13E"),
    ("CSpawn.cpp", "wm_field_140", "bm_field_140"),
    ("CSwitchCDStatus.cpp", "bm_field_0", "nm_field_0"),
    ("CUIControlBase.cpp", "nm_field_4", "bm_field_4"),
    ("CUIControlBase.cpp", "pm_field_36", "nm_field_36"),
    ("CUIControlEdit.cpp", "nm_field_872", "bm_field_872"),
    ("CVidMode.cpp", "wm_field_10", "nm_field_10"),
    ("CVisibility.cpp", "nm_field_4", "bm_field_4"),
]

SRC_DIR = "/home/wills/projects/IWD2-RE/iwd2-re/src/"

# Group by file for efficiency
from collections import defaultdict
by_file = defaultdict(list)
for cpp_file, old, new in MISMATCHES:
    by_file[cpp_file].append((old, new))

dry_run = "--apply" not in sys.argv
total_changes = 0

for cpp_file, renames in sorted(by_file.items()):
    path = SRC_DIR + cpp_file
    with open(path, 'r') as f:
        content = f.read()

    original = content
    for old, new in renames:
        # Use word-boundary regex to avoid substring matches
        # e.g., nm_field_2 should not match inside nm_field_28
        pattern = re.compile(r'\b' + re.escape(old) + r'\b')
        matches = pattern.findall(content)
        count = len(matches)
        if count == 0:
            print(f"  WARNING: {cpp_file}: '{old}' not found (already fixed?)")
            continue
        # Check that new name doesn't already exist (word-boundary)
        existing_pattern = re.compile(r'\b' + re.escape(new) + r'\b')
        existing = len(existing_pattern.findall(original))
        if existing > 0 and old != new:
            print(f"  WARNING: {cpp_file}: '{new}' already exists ({existing} times), skipping '{old}'→'{new}'")
            continue
        if dry_run:
            print(f"  {cpp_file}: '{old}' → '{new}' ({count} occurrences)")
        else:
            content = pattern.sub(new, content)
            print(f"  {cpp_file}: '{old}' → '{new}' ({count} occurrences) [APPLIED]")
        total_changes += count

    if not dry_run and content != original:
        with open(path, 'w') as f:
            f.write(content)

if dry_run:
    print(f"\n[DRY RUN] {total_changes} changes would be made. Run with --apply to apply.")
else:
    print(f"\n{total_changes} changes applied.")