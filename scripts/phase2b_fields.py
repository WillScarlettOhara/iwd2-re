#!/usr/bin/env python3
"""
Phase 2b: Auto-name field_X by analyzing decompiled functions and their field access patterns.
Cross-references offset accesses with semantic function names to infer field purposes.
"""
import json, re, os, subprocess, sys
from collections import defaultdict

TOKEN = "iwd2ee-re-tools-2026"
BASE = "http://127.0.0.1:8089"
SRC = "/home/wills/projects/IWD2-RE/iwd2-re/src"

def gh_get(path, params=""):
    url = f"{BASE}{path}?{params}"
    r = subprocess.run(["curl", "-sf", "-m", "60",
        "-H", f"Authorization: Bearer {TOKEN}", url],
        capture_output=True, text=True)
    return r.stdout.strip()

# Load field data
with open('/tmp/field_x_data.json') as f:
    fields = json.load(f)

# Group by class
by_class = defaultdict(list)
for fd in fields:
    by_class[fd['class']].append(fd)

print(f"Total classes: {len(by_class)}")

# =====================================================
# Strategy: For each class, get function variables + decompile
# to find how fields are used. Auto-suggest names.
# =====================================================

# First, let's focus on top classes
top_classes = sorted(by_class.items(), key=lambda x: -len(x[1]))[:10]
print(f"\nTop 10 classes to process:")
for cls, flds in top_classes:
    print(f"  {cls}: {len(flds)} field_X")

# For each class, get all its functions from Ghidra
# We need to decompile them and scan for field accesses

rename_map = {}  # (file, old_name) -> suggested_name

for cls_name, flds in top_classes[:5]:  # Process top 5
    print(f"\n--- Processing {cls_name} ---")
    
    # Get decompiled functions one by one for this class
    # We already have sub_ data - let's use that
    
    # Load sub_ results for this class
    with open('/tmp/phase2_results.json') as f:
        phase2 = json.load(f)
    
    class_funcs = [s for s in phase2.get('auto_named', []) 
                   if s.get('class_name') == cls_name or 
                   (not s.get('class_name') and cls_name in s.get('file', ''))]
    
    print(f"  Functions to analyze: {len(class_funcs)}")
    
    # For each field in this class, scan all functions for access patterns
    field_patterns = defaultdict(set)  # offset -> {seen_in_function, access_type}
    
    for func in class_funcs[:10]:  # Limit to first 10 per class for speed
        addr_hex = func['addr_hex']
        
        # Decompile this function
        decomp = gh_get("/decompile_function", f"address={addr_hex}")
        if not decomp:
            continue
        
        # Scan for field accesses: *(param_1 + 0xOFFSET) or *(this + 0xOFFSET)
        for m in re.finditer(r'\+\s*(0x([0-9A-Fa-f]+))\)\s*(=|==|!=)', decomp):
            offset_val = int(m.group(2), 16)
            access_type = 'write' if '=' in m.group(0) and '==' not in m.group(0) else 'read'
            field_patterns[offset_val].add(access_type)
        
        # Also: return *(param_1 + 0xOFFSET) -> getter
        for m in re.finditer(r'return\s+\*\([^)]+\+\s*(0x[0-9A-Fa-f]+)\)', decomp):
            offset_val = int(m.group(1), 16)
            field_patterns[offset_val].add('getter_return')
        
        # Also: FUN_xxx(param_1 + OFFSET) -> field passed to function
        for m in re.finditer(r'\([^)]*\+\s*(0x[0-9A-Fa-f]+)\)\s*\)', decomp):
            offset_val = int(m.group(1), 16)
            field_patterns[offset_val].add('passed_as_arg')
    
    # Now match field_X to these patterns
    matched = 0
    for fd in flds:
        off = fd['offset']
        if not off:
            continue
        
        patterns = field_patterns.get(off, set())
        if not patterns:
            continue
        
        matched += 1
        suggested = None
        
        # Look at function names that access this field
        # We named functions like GetXxx, SetXxx - extract the field name
        for func in class_funcs:
            new_name = func.get('new_name', '')
            if new_name and ('::Get' in new_name or '::Set' in new_name):
                # Extract what's being got/set
                parts = new_name.split('::')[-1]
                if parts.startswith('Get'):
                    suggested = parts[3:]  # Remove "Get"
                elif parts.startswith('Set'):
                    suggested = parts[3:]  # Remove "Set"
                if suggested:
                    break
        
        if not suggested:
            # Infer from type and patterns
            if 'getter_return' in patterns and 'write' not in patterns:
                suggested = f"m_{fd['type']}_{off:X}"
            elif 'write' in patterns and 'read' in patterns:
                suggested = f"m_field_{off:X}"
            elif 'passed_as_arg' in patterns:
                suggested = f"m_buffer_{off:X}"
            else:
                suggested = f"m_unk_{off:X}"
        
        if suggested:
            rename_map[(fd['class'], fd['file'], fd['name'])] = suggested
    
    print(f"  Fields with access patterns found: {matched}/{len(flds)}")
    print(f"  Suggested renames: {sum(1 for k in rename_map if k[0] == cls_name)}")


# Save rename map
out = []
for (cls, file, old), new in rename_map.items():
    out.append({'class': cls, 'file': file, 'old_name': old, 'new_name': new})

with open('/tmp/field_rename_map.json', 'w') as f:
    json.dump(out, f, indent=2)

print(f"\n\nTotal field renames suggested: {len(out)}")
print("Saved to /tmp/field_rename_map.json")

# Show some examples
print("\n--- Sample suggestions ---")
for r in out[:20]:
    print(f"  {r['class']}::{r['old_name']} -> {r['new_name']}")
