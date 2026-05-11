#!/usr/bin/env python3
"""
Phase 1: Automated discovery and scaffolding of missing functions.
3-pass approach:
  1. Categorize by address proximity to existing source files
  2. Batch decompile all truly missing functions
  3. Generate scaffold .h/.cpp entries
"""
import json, re, os, subprocess, time, sys
from collections import defaultdict

TOKEN = "iwd2ee-re-tools-2026"
BASE = "http://127.0.0.1:8089"
HEADERS = ["-H", f"Authorization: Bearer {TOKEN}"]
SRC_DIR = "/home/wills/projects/IWD2-RE/iwd2-re/src"

def curl(endpoint, method="GET", data=None, timeout=120):
    """Call Ghidra MCP endpoint via curl"""
    cmd = ["curl", "-sf", "-m", str(timeout), *HEADERS, f"{BASE}{endpoint}"]
    if method == "POST":
        cmd.extend(["-X", "POST", "-H", "Content-Type: application/json"])
        if data:
            cmd.extend(["-d", data])
    result = subprocess.run(cmd, capture_output=True, text=True)
    return result.stdout.strip()

def curl_json(endpoint, method="GET", data=None, timeout=120):
    """Call and parse JSON"""
    resp = curl(endpoint, method, data, timeout)
    try:
        return json.loads(resp)
    except:
        return resp

# ============================================================
# STEP 1: Map known functions to source files
# ============================================================
print("=" * 60)
print("STEP 1: Mapping known functions to source files...")
print("=" * 60)

with open('/tmp/iwd2_re_known_addrs.json') as fin:
    known_addrs = json.load(fin)

# Build: file -> list of addresses
file_to_addrs = defaultdict(list)
addr_to_file = {}
for hex_addr, info in known_addrs.items():
    addr = int(hex_addr, 16)
    path = info['file']
    file_to_addrs[path].append((addr, info.get('name', 'unknown')))
    addr_to_file[addr] = path

# Build address ranges for each file
file_ranges = {}
for fpath, addrs in file_to_addrs.items():
    if addrs:
        sorted_addrs = sorted(a[0] for a in addrs)
        file_ranges[fpath] = (sorted_addrs[0], sorted_addrs[-1], sorted_addrs)

print(f"  {len(file_to_addrs)} source files with known functions")

# ============================================================
# STEP 2: Load missing functions
# ============================================================
print("\n" + "=" * 60)
print("STEP 2: Loading missing functions data...")
print("=" * 60)

with open('/tmp/missing_functions.json') as fin2:
    missing_data = json.load(fin2)

missing_funcs = missing_data['functions']
print(f"  {len(missing_funcs)} truly missing functions to scaffold")

# ============================================================
# STEP 3: Categorize by address proximity
# ============================================================
print("\n" + "=" * 60)
print("STEP 3: Categorizing by address proximity...")
print("=" * 60)

# For each missing function, find the known function with the closest address
categorized = defaultdict(list)  # file -> [(addr, name)]
uncategorized = []

# Build sorted list of all known (addr, file) pairs
all_known = sorted([(addr, file) for addr, file in addr_to_file.items()])

import bisect
known_addrs_sorted = [a[0] for a in all_known]
known_files = [a[1] for a in all_known]

for func in missing_funcs:
    addr = func['addr']
    # Binary search for nearest
    idx = bisect.bisect_left(known_addrs_sorted, addr)
    
    best_dist = float('inf')
    best_file = None
    
    # Check neighbors
    for offset in range(-3, 4):
        check_idx = idx + offset
        if 0 <= check_idx < len(known_addrs_sorted):
            dist = abs(known_addrs_sorted[check_idx] - addr)
            if dist < best_dist:
                best_dist = dist
                best_file = known_files[check_idx]
    
    if best_dist <= 0x1000 and best_file:  # Within 4KB - same module
        # Find the class prefix from nearby functions
        class_prefix = None
        for check_idx in range(max(0, idx-5), min(len(known_addrs_sorted), idx+5)):
            kaddr = known_addrs_sorted[check_idx]
            kfile = known_files[check_idx]
            # Extract class from nearby function names
            name = known_addrs.get(str(hex(kaddr)), {}).get('name', '')
            if '::' in name:
                class_prefix = name.split('::')[0]
                break
        
        categorized[best_file].append({
            'addr': addr,
            'name': func['name'],
            'distance': best_dist,
            'class_hint': class_prefix
        })
    else:
        uncategorized.append(func)

total_categorized = sum(len(v) for v in categorized.values())
print(f"  Categorized: {total_categorized} functions across {len(categorized)} files")
print(f"  Uncategorized: {len(uncategorized)} functions (will batch decompile first)")

# ============================================================
# STEP 4: Handle uncategorized — batch decompile
# ============================================================
if uncategorized:
    print("\n" + "=" * 60)
    print(f"STEP 4: Batch decompiling {len(uncategorized)} uncategorized functions...")
    print("=" * 60)
    
    decomp_results = {}
    batch_size = 20
    
    for i in range(0, len(uncategorized), batch_size):
        batch = uncategorized[i:i+batch_size]
        batch_addrs = ",".join(hex(f['addr']) for f in batch)
        
        sys.stdout.write(f"\r  Batch {i//batch_size + 1}/{((len(uncategorized)-1)//batch_size)+1} ({len(batch)} funcs)...")
        sys.stdout.flush()
        
        resp = curl(f"/batch_decompile?functions={batch_addrs}", timeout=180)
        if resp:
            for bf in batch:
                decomp_results[bf['addr']] = resp
        time.sleep(0.3)
    
    print(f"\n  Done. {len(decomp_results)} decompiled.")

# ============================================================
# STEP 5: Generate scaffold
# ============================================================
print("\n" + "=" * 60)
print("STEP 5: Generating scaffolding...")
print("=" * 60)

scaffold_dir = "/home/wills/projects/IWD2-RE/iwd2-re/scaffold"
os.makedirs(scaffold_dir, exist_ok=True)

# Summary of what we'll generate
summary = {
    'categorized_files': len(categorized),
    'categorized_functions': total_categorized,
    'uncategorized': len(uncategorized),
    'files_to_create': [],
}

# For each file with missing functions, create scaffold additions
for fpath, funcs in sorted(categorized.items()):
    rel_path = fpath
    base_name = os.path.splitext(os.path.basename(rel_path))[0]
    is_header = fpath.endswith('.h')
    
    scaffold_file = os.path.join(scaffold_dir, f"{base_name}_additions.{'h' if is_header else 'cpp'}")
    
    with open(scaffold_file, 'w') as f:
        f.write(f"// Scaffold additions for {rel_path}\n")
        f.write(f"// {len(funcs)} new functions to add\n")
        f.write(f"// Generated by Phase 1 scaffold script\n\n")
        
        for func in sorted(funcs, key=lambda x: x['addr']):
            addr = func['addr']
            dist = func['distance']
            hint = func.get('class_hint', '')
            
            f.write(f"// TODO: Scaffold - {hex(addr)} (distance {dist} from nearest known)\n")
            if hint:
                f.write(f"// Belongs to class: {hint}\n")
            f.write(f"// 0x{addr:06X}\n")
            f.write(f"void sub_{addr:06X}();\n\n" if is_header else 
                    f"// 0x{addr:06X}\nvoid sub_{addr:06X}() {{\n    // TODO: Incomplete.\n}}\n\n")
    
    summary['files_to_create'].append(f"{base_name}_additions.{'h' if is_header else 'cpp'}")
    print(f"  Created {scaffold_file} ({len(funcs)} functions)")

# Create uncategorized scaffold
if uncategorized:
    uncat_file = os.path.join(scaffold_dir, "uncategorized_new.h")
    with open(uncat_file, 'w') as f:
        f.write("// Uncategorized newly discovered functions\n")
        f.write(f"// {len(uncategorized)} functions — needs manual classification\n\n")
        for func in sorted(uncategorized, key=lambda x: x['addr']):
            f.write(f"// 0x{func['addr']:06X}\n")
            f.write(f"void FUN_{func['addr']:06X}();\n\n")
    print(f"  Created {uncat_file} ({len(uncategorized)} functions)")
    summary['files_to_create'].append("uncategorized_new.h")

# Save summary
with open('/tmp/phase1_summary.json', 'w') as f:
    json.dump(summary, f, indent=2)

print(f"\n=== Phase 1 Summary ===")
print(f"  Categorized: {total_categorized} functions in {len(categorized)} files")
print(f"  Uncategorized: {len(uncategorized)} functions")
print(f"  Scaffold dir: {scaffold_dir}")
print(f"  Full summary: /tmp/phase1_summary.json")
