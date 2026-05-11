#!/usr/bin/env python3
"""
Phase 5: Complete field catalog builder.

Extracts all class field declarations (field_X) from headers, groups by class,
and assigns priority based on class size and function completeness.

Strategy:
1. Small classes with complete implementations = easy wins
2. Struct fields with array-like patterns = likely indices/counts
3. CGameSprite and massive classes = use Ghidra MCP for targeted decompilation
"""
import json, re, os
from collections import defaultdict, Counter

SRC = "/home/wills/projects/IWD2-RE/iwd2-re/src"

def build_catalog():
    class_fields = defaultdict(list)
    field_usage = Counter()
    
    for root, dirs, files in os.walk(SRC):
        for file in files:
            if not file.endswith('.h'):
                continue
            filepath = os.path.join(root, file)
            try:
                with open(filepath, 'r') as f:
                    content = f.read()
            except Exception:
                continue
            
            # Extract current class name
            class_name = None
            for m in re.finditer(r'(class|struct)\s+(\w+)', content):
                class_name = m.group(2)
            
            if not class_name:
                continue
            
            # Find field declarations
            for m in re.finditer(
                r'(/\*\s*(0x[0-9A-Fa-f]+|[0-9A-Fa-f]+)\s*\*/)?\s*' 
                + r'(.*?)\b([a-zA-Z_]*)field_([0-9A-Fa-f_]+)\b\s*;',
                content
            ):
                offset = m.group(2) if m.group(2) else ''
                dtype = m.group(3).strip() if m.group(3) else ''
                prefix = m.group(4)
                offset_num = m.group(5)
                field_name = f"{prefix}field_{offset_num}"
                
                class_fields[class_name].append({
                    'field_name': field_name,
                    'offset': offset,
                    'type': dtype,
                    'prefix': prefix,
                    'offset_num': offset_num
                })
    
    return class_fields

def infer_from_type_and_offset(fields):
    """Heuristic: infer some names from type + common patterns."""
    inferred = {}
    for f in fields:
        t = f['type'].lower()
        on = f['offset_num'].lower()
        
        # DWORD at offset 0 with pointer-like name = likely base/flags
        if on == '0' and 'dword' in t:
            inferred[f['field_name']] = 'm_dwFlags'
        
        # WORD at offset 0 = likely count or type
        if on == '0' and ('word' in t or 'short' in t):
            inferred[f['field_name']] = 'm_wType'
        
        # BYTE at offset 0 = flags or type
        if on == '0' and ('byte' in t or 'char' in t):
            inferred[f['field_name']] = 'm_bFlags'
    
    return inferred

def main():
    catalog = build_catalog()
    
    # Sort classes by field count ascending (easy first)
    sorted_classes = sorted(catalog.items(), key=lambda x: len(x[1]))
    
    print("=== All Classes with field_X members (sorted by field count) ===\n")
    
    easy_wins = []
    medium = []
    hard = []
    
    for cls, fields in sorted_classes:
        count = len(fields)
        total_offset_span = 0
        for f in fields:
            try:
                off = int(f['offset'], 16) if f['offset'].startswith('0x') else int(f['offset'], 16)
            except:
                off = 0
            total_offset_span = max(total_offset_span, off)
        
        if count <= 5:
            easy_wins.append((cls, count, fields))
        elif count <= 20:
            medium.append((cls, count, total_offset_span, fields))
        else:
            hard.append((cls, count, total_offset_span, fields))
    
    print(f"\n### EASY WINS ({len(easy_wins)} classes, <=5 fields each)")
    for cls, count, fields in easy_wins[:15]:
        print(f"\n{cls} ({count} fields)")
        for f in fields:
            print(f"  {f['offset']:8s} {f['type']:25s} {f['field_name']}")
    
    print(f"\n### MEDIUM ({len(medium)} classes, 6-20 fields)")
    for cls, count, span, fields in medium[:10]:
        print(f"\n{cls} ({count} fields, span 0x{span:x})")
        for f in fields[:5]:
            print(f"  {f['offset']:8s} {f['type']:25s} {f['field_name']}")
    
    print(f"\n### HARD ({len(hard)} classes, >20 fields)")
    for cls, count, span, fields in hard:
        print(f"\n{cls} ({count} fields, span 0x{span:x})")
        for f in fields[:3]:
            print(f"  {f['offset']:8s} {f['type']:25s} {f['field_name']}")
    
    print(f"\n=== Summary ===")
    print(f"Easy classes: {len(easy_wins)}")
    print(f"Medium classes: {len(medium)}")
    print(f"Hard classes: {len(hard)}")
    total_fields = sum(len(v) for v in catalog.values())
    print(f"Total fields to name: {total_fields}")

if __name__ == '__main__':
    main()
