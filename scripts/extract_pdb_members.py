#!/usr/bin/env python3
"""Extract class member data from BG2EE PDB symbols.

Usage:
    llvm-pdbutil dump -types /path/to/Baldur.pdb > /tmp/bg2_pdb_types.txt
    python3 extract_pdb_members.py /tmp/bg2_pdb_types.txt > /tmp/bg2_pdb_members.txt
"""

import re
import sys

def extract_members(pdb_dump_path):
    # Step 1: Build class -> field_list_id mapping (non-forward-ref only)
    class_fl = {}
    with open(pdb_dump_path) as f:
        content = f.read()
    for m in re.finditer(
        r'0x([0-9A-F]+)\s*\|\s*LF_CLASS\s+\[.*?\]\s+\x60(.+?)\x60.*?field\s+list:\s*0x([0-9A-F]+)',
        content, re.DOTALL
    ):
        cls_id, cls_name, fl_id = m.group(1), m.group(2), m.group(3)
        chunk = content[m.start():m.start()+500]
        if 'forward ref' not in chunk:
            class_fl[cls_name] = fl_id

    # Step 2: Parse field lists into {fl_id: [(name, type, offset)]}
    fl_members = {}
    current_fl = None
    current_members = []
    with open(pdb_dump_path) as f:
        for line in f:
            m = re.match(r'\s*0x([0-9A-F]+)\s*\|\s*LF_FIELDLIST', line)
            if m:
                if current_fl and current_members:
                    fl_members[current_fl] = current_members
                current_fl = m.group(1).upper()
                current_members = []

            m2 = re.search(
                r'LF_MEMBER\s+\[name\s+=\s+\x60(.+?)\x60,'
                r'\s+Type\s+=\s+0x[0-9A-F]+\s+\(([^)]*)\),'
                r'\s+offset\s+=\s+(\d+)',
                line
            )
            if m2 and current_fl:
                current_members.append((m2.group(1), m2.group(2), int(m2.group(3))))

        if current_fl and current_members:
            fl_members[current_fl] = current_members

    # Step 3: Output relevant classes
    engine_prefixes = [
        'CGame', 'CInf', 'CChitin', 'CInfinity', 'CRender', 'CVid', 'CUI',
        'CSpawning', 'CSpawn', 'CTrigger', 'CContainer', 'CEffect', 'CScreen',
        'CMessage', 'CButton', 'CAbility', 'CCache', 'CRes', 'CAI',
        'CDerivedStats', 'CColor', 'CVariable', 'CWorldMap', 'CPathSearch',
        'CSearchBitmap', 'CFont', 'CSound', 'CMusic', 'CMovie',
        'CTlkFile', 'CKeyFile', 'CBifFile', 'CAreFile', 'CFile',
        'CBitmap', 'CPolygon', 'CMap', 'CAreaFile', 'CItem',
        'CProjectile', 'CSpell', 'CStore', 'CDialog', 'CScript',
    ]
    for cls_name in sorted(class_fl.keys()):
        fl_id = class_fl[cls_name]
        if not any(cls_name.startswith(p) for p in engine_prefixes):
            continue
        members = fl_members.get(fl_id, [])
        if not members:
            continue
        print(f"\n=== {cls_name} (fl=0x{fl_id}, {len(members)} members) ===")
        for mname, mtype, moff in sorted(members, key=lambda x: x[2]):
            print(f"  0x{moff:04X} {mtype:30s} {mname}")


if __name__ == '__main__':
    if len(sys.argv) < 2:
        print(f"Usage: {sys.argv[0]} <pdb_types_dump.txt>", file=sys.stderr)
        sys.exit(1)
    extract_members(sys.argv[1])