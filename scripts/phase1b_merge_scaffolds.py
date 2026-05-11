#!/usr/bin/env python3
"""
Merge scaffolds: fetch functions from Ghidra (with Phase 2 names),
cross-ref against source, insert missing declarations/definitions
into existing .h/.cpp files.
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

print("Fetching all functions from Ghidra (with Phase 2 names)...")
ghidra = []
for line in gh_get("/list_functions").split('\n'):
    if ' at ' in line and len(line.split(' at ')) == 2:
        name, addr = line.split(' at ')
        ghidra.append({'name': name.strip(), 'addr': int(addr.strip(), 16)})

print(f"  {len(ghidra)} functions total")

# Load known addresses
with open('/tmp/iwd2_re_known_addrs.json') as f:
    known = json.load(f)
known_set = {int(k, 16) for k in known}

# Find functions in Ghidra NOT in source (code range only)
missing = [g for g in ghidra if 0x401000 <= g['addr'] <= 0x7FFFFF and g['addr'] not in known_set]
print(f"  Missing from source: {len(missing)}")

# Categorize each missing function to nearest source file
import bisect
known_sorted = sorted(int(k, 16) for k in known)
addr_to_file = {int(k, 16): v['file'] for k, v in known.items()}

by_file = defaultdict(list)
uncategorized = []

for func in missing:
    addr = func['addr']
    idx = bisect.bisect_left(known_sorted, addr)
    
    best_dist = float('inf')
    best_file = None
    for off in range(-3, 4):
        ci = idx + off
        if 0 <= ci < len(known_sorted):
            d = abs(known_sorted[ci] - addr)
            if d < best_dist:
                best_dist = d
                best_file = addr_to_file.get(known_sorted[ci])
    
    if best_dist <= 0x4000 and best_file:
        by_file[best_file].append({**func, 'dist': best_dist})
    else:
        uncategorized.append(func)

print(f"  Categorized: {sum(len(v) for v in by_file.values())} in {len(by_file)} files")
print(f"  Uncategorized: {len(uncategorized)}")

# Insert declarations into headers and cpp files
inserted = 0
for fpath, funcs in sorted(by_file.items()):
    full = os.path.join(SRC, fpath)
    if not os.path.exists(full):
        continue
    
    with open(full) as f:
        content = f.read()
    
    # Determine if header or cpp
    is_header = fpath.endswith('.h')
    base = os.path.splitext(os.path.basename(fpath))[0]
    
    # Read companion file
    companion = None
    if is_header:
        cpp_path = os.path.join(SRC, f"{base}.cpp")
        if os.path.exists(cpp_path):
            with open(cpp_path) as f:
                companion = f.read()
        music_path = os.path.join(SRC, 'music', f"{base}.cpp")
        if os.path.exists(music_path):
            with open(music_path) as f:
                companion = f.read()
    
    additions_h = []
    additions_cpp = []
    
    for func in sorted(funcs, key=lambda x: x['addr']):
        name = func['name']
        addr = func['addr']
        
        # Skip if already declared/defined
        if re.search(rf'\b{re.escape(name)}\b\s*\(', content):
            continue
        if companion and re.search(rf'\b{re.escape(name)}\b\s*\(', companion):
            continue
        
        # Determine if it's a class method
        class_name = None
        if '::' in name:
            class_name = name.split('::')[0]
        
        if is_header and class_name and class_name == base:
            # Class method declaration
            additions_h.append(f"    void {name.split('::')[-1]}(); // 0x{addr:06X}\n")
        elif is_header:
            # Free function declaration
            additions_h.append(f"void {name}(); // 0x{addr:06X}\n")
        else:
            # cpp file - stub definition
            additions_cpp.append(f"// 0x{addr:06X}\nvoid {name}() {{\n    // TODO: Incomplete.\n}}\n\n")
    
    if additions_h:
        # Insert before last #endif or at end of class
        if is_header and base in content:
            # Find the closing }; of the class
            class_end = content.rfind('};')
            if class_end > 0:
                content = content[:class_end] + '\n'.join(additions_h) + '\n' + content[class_end:]
                inserted += len(additions_h)
        else:
            # Just append at end before any #endif
            endif_pos = content.rfind('#endif')
            if endif_pos > 0:
                content = content[:endif_pos] + '\n// Phase 1-2: Newly discovered functions\n' + ''.join(additions_h) + '\n' + content[endif_pos:]
            else:
                content += '\n// Phase 1-2: Newly discovered functions\n' + ''.join(additions_h)
            inserted += len(additions_h)
        
        with open(full, 'w') as f:
            f.write(content)
    
    if additions_cpp:
        cpp_full = os.path.join(SRC, f"{base}.cpp")
        if not os.path.exists(cpp_full):
            cpp_full = os.path.join(SRC, 'music', f"{base}.cpp")
        if os.path.exists(cpp_full):
            with open(cpp_full) as f:
                cpp_content = f.read()
            cpp_content += '\n// Phase 1-2: Scaffold functions\n' + ''.join(additions_cpp)
            with open(cpp_full, 'w') as f:
                f.write(cpp_content)
            inserted += len(additions_cpp)

print(f"\nTotal declarations/definitions inserted: {inserted}")

# Uncategorized
uncat_path = os.path.join(SRC, "NewDiscovered.h")
with open(uncat_path, 'w') as f:
    f.write("// Newly discovered functions — not yet categorized\n")
    f.write(f"// {len(uncategorized)} functions\n")
    f.write("// Group by address proximity to determine which source file they belong to\n\n")
    
    # Group by address range
    ranges = {}
    for func in sorted(uncategorized, key=lambda x: x['addr']):
        hi = (func['addr'] >> 16) & 0xFF
        if hi not in ranges:
            ranges[hi] = []
        ranges[hi].append(func)
    
    for hi in sorted(ranges):
        funcs = ranges[hi]
        f.write(f"// === 0x{hi:02X}0000 range ===\n")
        for func in funcs:
            f.write(f"void {func['name']}(); // 0x{func['addr']:06X}\n")
        f.write('\n')

print(f"Uncategorized saved to {uncat_path}")

# Final count of field_ references
field_count = 0
for root, dirs, files in os.walk(SRC):
    for fname in files:
        if fname.endswith(('.h', '.cpp')):
            path = os.path.join(root, fname)
            with open(path, errors='ignore') as f:
                field_count += len(re.findall(r'\bfield_\w+\b', f.read()))
print(f"\nRemaining field_ references: {field_count}")
