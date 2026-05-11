#!/usr/bin/env python3
"""
Phase 2b: Deep field analysis for CGameSprite (108 field_X).
Cross-references decompiled function bodies with header field offsets
to produce semantic field names.
"""
import json, re, subprocess, sys
from collections import defaultdict

TOKEN = "iwd2ee-re-tools-2026"
BASE = "http://127.0.0.1:8089"

def gh_get(path, params=""):
    url = f"{BASE}{path}?{params}"
    r = subprocess.run(["curl", "-sf", "-m", "60",
        "-H", f"Authorization: Bearer {TOKEN}", url],
        capture_output=True, text=True)
    return r.stdout.strip()

print("Loading data...")

# CGameSprite field_X from header
with open('/tmp/field_x_data.json') as f:
    all_fields = json.load(f)

cgs_fields = [f for f in all_fields if f['class'] == 'CGameSprite']
cgs_field_offsets = {f['offset']: f for f in cgs_fields if f['offset']}
print(f"CGameSprite field_X: {len(cgs_fields)} (with offsets: {len(cgs_field_offsets)})")

# Our renames for CGameSprite
with open('/tmp/cgs_rename_mapping.json') as f:
    cgs_renames = json.load(f)

func_names = {r[1]: (r[2], r[3]) for r in cgs_renames['renames']}

# Now for each renamed function, decompile and extract field accesses
print("\nDecompiling CGameSprite functions to extract field access patterns...")

# Map: offset -> {set_by: [functions], read_by: [functions]}
field_access = defaultdict(lambda: {'read': [], 'write': [], 'test': [], 'return': []})

for i, (addr_hex, old_name, new_name, rationale) in enumerate(cgs_renames['renames']):
    decomp = gh_get("/decompile_function", f"address={addr_hex}")
    if not decomp:
        continue
    
    short_name = new_name.split('::')[-1] if '::' in new_name else new_name
    
    # Extract offsets accessed
    # Pattern: *(param_1 + 0xXXXX) = value  -> write
    # Pattern: if (*(param_1 + 0xXXXX) ...) -> read/test
    # Pattern: return *(param_1 + 0xXXXX)    -> return/getter
    
    for m in re.finditer(r'\+\s*(\d+)\)\s*=\s', decomp):
        off = int(m.group(1))
        field_access[off]['write'].append(short_name)
    
    for m in re.finditer(r'\+\s*(\d+)\)\s*\)', decomp):
        off = int(m.group(1))
        field_access[off]['read'].append(short_name)
    
    for m in re.finditer(r'return\s+\*\(.*?\+\s*(\d+)\)', decomp):
        off = int(m.group(1))
        field_access[off]['return'].append(short_name)
    
    for m in re.finditer(r'return\s+~?\(?\(?byte\)?\*\(.*?\+\s*(\d+)\)', decomp):
        off = int(m.group(1))
        field_access[off]['return'].append(short_name)
    
    # Also offset used directly: *(int*)(param_1 + OFFSET) without assignment
    for m in re.finditer(r'\+\s*(0x[0-9A-Fa-f]+)\)(?!=)', decomp):
        off = int(m.group(1), 16)
        if short_name not in field_access[off]['read'] and short_name not in field_access[off]['write']:
            field_access[off]['read'].append(short_name)
    
    sys.stdout.write(f"\r  {i+1}/{len(cgs_renames['renames'])} ")
    sys.stdout.flush()

print(f"\n\nTotal offsets accessed: {len(field_access)}")

# Now generate names for each field_X based on what accesses it
print("\n--- Field naming suggestions ---")
suggestions = {}

for off, fd in cgs_field_offsets.items():
    access = field_access.get(off, {'read': [], 'write': [], 'return': [], 'test': []})
    
    # Priority: if a function named SetXxx writes to this offset, the field is Xxx
    name = None
    
    # Check write functions for SetXxx pattern
    for func in access['write']:
        if func.startswith('Set'):
            name = func[3:]  # Remove "Set"
            break
    
    # Check return/getter functions for GetXxx pattern
    if not name:
        for func in access['return']:
            if func.startswith('Get'):
                name = func[3:]
                break
    
    # Check read functions for hints
    if not name:
        for func in (access['read'] + access['return']):
            if func.startswith('Is') or func.startswith('Has') or func.startswith('Can'):
                name = func  # Keep full name as hint
                break
    
    if name:
        # Convert PascalCase to camelCase for member
        if name[0].isupper():
            name = name[0].lower() + name[1:]
    else:
        # Fallback heuristic
        ftype = fd['type']
        if ftype in ('INT', 'int', 'DWORD', 'uint', 'UINT', 'LONG', 'long'):
            prefix = 'n'
        elif ftype in ('SHORT', 'short', 'WORD', 'USHORT'):
            prefix = 'w'
        elif ftype in ('BYTE', 'byte', 'CHAR', 'char', 'BOOLEAN', 'BOOL', 'bool'):
            prefix = 'b'
        elif 'PTR' in ftype or ftype.endswith('*'):
            prefix = 'p'
        elif ftype in ('CString', 'CResRef', 'RESREF'):
            prefix = 's'
        else:
            prefix = 'c'
        name = f"field_{off:X}"
    
    suggestions[off] = {
        'old': fd['name'],
        'new': f"m_{name}" if not name.startswith('m_') else name,
        'type': fd['type'],
        'access': {k: list(set(v)) for k, v in access.items() if v},
        'line': fd['line'],
    }
    
    # Print
    access_str = ", ".join(f"{k}: {', '.join(v[:3])}" for k, v in access.items() if v)
    if access_str:
        print(f"  0x{off:04X}  {fd['name']} -> {suggestions[off]['new']}  [{access_str}]")
    else:
        print(f"  0x{off:04X}  {fd['name']} -> {suggestions[off]['new']}  [no access detected]")

# Apply renames to source file
print(f"\nApplying renames to CGameSprite.h...")
header_path = "/home/wills/projects/IWD2-RE/iwd2-re/src/CGameSprite.h"
with open(header_path) as f:
    content = f.read()

renamed = 0
for off, sug in suggestions.items():
    old = sug['old']
    new = sug['new']
    if old != new:
        # Replace in content (word boundary to avoid partial matches)
        content = re.sub(r'\b' + re.escape(old) + r'\b', new, content)
        renamed += 1

with open(header_path, 'w') as f:
    f.write(content)

print(f"Applied {renamed} field renames to CGameSprite.h")

# Save suggestions
with open('/tmp/cgs_field_suggestions.json', 'w') as f:
    json.dump({f"0x{off:04X}": sug for off, sug in suggestions.items()}, f, indent=2, default=list)

print(f"Detailed suggestions saved to /tmp/cgs_field_suggestions.json")
