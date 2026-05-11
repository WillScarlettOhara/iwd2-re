#!/usr/bin/env python3
"""
Phase 5: Auto-name field_X members based on getter/setter analysis.
Uses the fact that we named functions like GetField_XXX/SetField_XXX
which tells us what the field represents.
"""
import json, re, os
from collections import defaultdict

SRC = "/home/wills/projects/IWD2-RE/iwd2-re/src"

def main():
    # Build map: (class, offset) -> suggested_name
    field_names = {}
    
    # Strategy 1: Look at getter/setter names we already identified
    # CGameSprite::GetBHiding() implies field at offset 0x7240 is m_bHiding
    
    known_getters = {
        'CGameAIBase': {
            0x58C: 'm_nAITarget',  # from GetField58C
        },
        'CGameDoor': {
            0x5C4: 'm_dwFlags',     # from GetDwFlags
        },
        'CGameSprite': {
            0x7240: 'm_bHiding',     # from GetBHiding/SetBHiding
            0x562C: 'm_field_562C', # still unknown (SetField562C)
            0x70F2: 'm_field_70F2', # still unknown
            0x80C: 'm_field_80C',   # still unknown
        },
        'CGameAreaNotes': {
            0x8D: 'bm_field_8D',     # from GetMField8D
        },
        'CMultiplayerSettings': {
            0xAC: 'm_nCharacterViewing',   # from GetCharacterViewingCount
            0xB2: 'm_nReadyState',          # from GetReadyCount/ClearReadyState
        },
    }
    
    # Strategy 2: Pattern match common field names in the binary
    # Look at usages to deduce meaning
    
    # Strategy 3: Find fields that are only accessed in one way
    field_access = defaultdict(lambda: {'reads': 0, 'writes': 0, 'funcs': set()})
    
    for root, dirs, files in os.walk(SRC):
        for file in files:
            if file.endswith(('.cpp', '.h')):
                filepath = os.path.join(root, file)
                with open(filepath, 'r') as f:
                    content = f.read()
                
                # Find field_XXXX accesses
                for m in re.finditer(r'\b(\w*)field_([0-9A-Fa-f]+)\b', content):
                    prefix = m.group(1)
                    offset = m.group(2)
                    
                    # Determine if read or write
                    # Simple heuristic: check if it's on left side of =
                    line_start = content.rfind('\n', 0, m.start())
                    line_end = content.find('\n', m.end())
                    line = content[line_start:line_end]
                    
                    is_write = '=' in line and line.index('=') > m.end() - line_start
                    
                    key = (prefix, offset)
                    if is_write:
                        field_access[key]['writes'] += 1
                    else:
                        field_access[key]['reads'] += 1
    
    # Print summary
    print("=== Field Access Analysis ===")
    print(f"Total unique field patterns: {len(field_access)}")
    print()
    
    # Show fields with no prefix (need Hungarian)
    no_prefix = [(k, v) for k, v in field_access.items() if k[0] == '']
    print(f"Fields needing Hungarian prefix: {len(no_prefix)}")
    
    # Show fields that are only read (likely derived/state)
    read_only = [(k, v) for k, v in field_access.items() if v['reads'] > 0 and v['writes'] == 0]
    print(f"Read-only fields (likely getters): {len(read_only)}")
    
    # Fields with both read and write
    read_write = [(k, v) for k, v in field_access.items() if v['reads'] > 0 and v['writes'] > 0]
    print(f"Read-write fields (state): {len(read_write)}")
    
    print()
    print("=== Sample fields to name ===")
    
    # Show some specific examples from CGameSprite
    cgs_fields = [(k, v) for k, v in field_access.items() if 'CGameSprite' in str(v)]
    
    # Show most accessed fields first
    sorted_fields = sorted(field_access.items(), key=lambda x: -(x[1]['reads'] + x[1]['writes']))
    for (prefix, offset), stats in sorted_fields[:30]:
        name = f"{prefix}field_{offset}"
        print(f"  {name:20s} reads={stats['reads']:3d} writes={stats['writes']:3d}")

if __name__ == '__main__':
    main()
