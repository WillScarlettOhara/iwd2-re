#!/usr/bin/env python3
"""Phase 1: Cross-reference Ghidra functions with iwd2-re known addresses."""
import json, requests, sys, time

TOKEN = "iwd2ee-re-tools-2026"
BASE = "http://127.0.0.1:8089"
HEADERS = {"Authorization": f"Bearer {TOKEN}"}

print("Loading known addresses from source...")
with open('/tmp/iwd2_re_known_addrs.json') as f:
    known_addrs = json.load(f)
known_addr_set = {int(k, 16) for k in known_addrs}
print(f"  Known addresses from source: {len(known_addr_set)}")

print("Fetching all functions from Ghidra...")
all_functions = []
offset = 0
limit = 500
while True:
    try:
        resp = requests.get(f"{BASE}/list_functions",
                            headers=HEADERS,
                            params={"offset": offset, "limit": limit},
                            timeout=30)
    except requests.Timeout:
        print(f"  Timeout at offset {offset}, retrying...")
        time.sleep(2)
        continue

    text = resp.text.strip()
    if not text:
        break

    lines = text.split('\n')
    batch = []
    for line in lines:
        if ' at ' in line:
            parts = line.split(' at ')
            if len(parts) == 2:
                name = parts[0].strip()
                try:
                    addr = int(parts[1].strip(), 16)
                    batch.append({'name': name, 'addr': addr})
                except ValueError:
                    pass

    all_functions.extend(batch)
    sys.stdout.write(f"\r  Fetched {len(all_functions)} functions (offset {offset})")
    sys.stdout.flush()

    offset += limit
    if len(batch) < limit * 0.9:  # Got fewer than expected - probably last page
        break
    time.sleep(0.2)

print(f"\n  Total functions in Ghidra: {len(all_functions)}")

print("Cross-referencing...")
ghidra_addr_set = {f['addr'] for f in all_functions}
matched = known_addr_set & ghidra_addr_set
only_source = known_addr_set - ghidra_addr_set
only_ghidra = ghidra_addr_set - known_addr_set

only_ghidra_code = {a for a in only_ghidra if 0x401000 <= a <= 0x7FFFFF}
missing_funcs = [f for f in all_functions if f['addr'] in only_ghidra_code]
unnamed_missing = [f for f in missing_funcs if f['name'].startswith(('FUN_', 'sub_'))]

print(f"\n=== Cross Reference Results ===")
print(f"  Matched (in both):     {len(matched)}")
print(f"  Only in source:        {len(only_source)}")
print(f"  Only in Ghidra (code): {len(only_ghidra_code)}")
print(f"  -> Unnamed (FUN_/sub_): {len(unnamed_missing)}")
print(f"  -> Already named:      {len(missing_funcs) - len(unnamed_missing)}")

# Stats about the source
sub_only = sum(1 for v in known_addrs.values() if v['name'].startswith('sub_'))
named_source = len(known_addrs) - sub_only
print(f"\n=== Source Code Stats ===")
print(f"  Named functions:       {named_source}")
print(f"  sub_ functions:        {sub_only}")

# Save all data
output = {
    'stats': {
        'known_source': len(known_addr_set),
        'ghidra_total': len(all_functions),
        'matched': len(matched),
        'only_source': len(only_source),
        'only_ghidra_code': len(only_ghidra_code),
        'unnamed_missing': len(unnamed_missing),
        'named_missing': len(missing_funcs) - len(unnamed_missing),
        'named_source': named_source,
        'sub_source': sub_only,
    },
    'missing_functions': missing_funcs[:2000],  # First 2000 for initial work
    'missing_count': len(missing_funcs),
}

with open('/tmp/phase1_results.json', 'w') as f:
    json.dump(output, f, indent=2)

print(f"\nResults saved to /tmp/phase1_results.json")
print(f"Missing functions to scaffold: {len(missing_funcs)}")
print(f"First step: scaffold the {len(unnamed_missing)} unnamed ones")
