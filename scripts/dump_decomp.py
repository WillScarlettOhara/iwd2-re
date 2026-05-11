#!/usr/bin/env python3
"""
Dump all decompiled code for TODO: Incomplete functions into reference files.
"""
import re, os, subprocess, sys, time

TOKEN = "iwd2ee-re-tools-2026"
BASE = "http://127.0.0.1:8089"
SRC = "/home/wills/projects/IWD2-RE/iwd2-re/src"
OUT = "/home/wills/projects/IWD2-RE/iwd2-re/decomp_ref"

def gh_get(path, params=""):
    url = f"{BASE}{path}?{params}"
    r = subprocess.run(["curl", "-sf", "-m", "60",
        "-H", f"Authorization: Bearer {TOKEN}", url],
        capture_output=True, text=True)
    return r.stdout.strip()

os.makedirs(OUT, exist_ok=True)

# Find ALL incomplete functions with their addresses
addr_pattern = re.compile(r'// 0x([0-9A-Fa-f]{6,8})')
incomplete_files = {}

for root, dirs, files in os.walk(SRC):
    for fname in files:
        if not fname.endswith(('.cpp', '.h')):
            continue
        path = os.path.join(root, fname)
        rel = os.path.relpath(path, SRC)
        with open(path, errors='ignore') as f:
            lines = f.readlines()
        
        for i, line in enumerate(lines):
            if '// TODO: Incomplete' not in line and 'Incomplete.' not in line:
                continue
            # Look back up to 3 lines for address comment
            addr = None
            for lookback in range(1, 4):
                if i - lookback >= 0:
                    m = addr_pattern.search(lines[i - lookback])
                    if m:
                        addr = int(m.group(1), 16)
                        break
            if addr:
                if rel not in incomplete_files:
                    incomplete_files[rel] = []
                incomplete_files[rel].append(addr)

total = sum(len(v) for v in incomplete_files.values())
print(f"Total incomplete functions: {total} across {len(incomplete_files)} files")

# Decompile all and save
processed = 0
for rel, addrs in sorted(incomplete_files.items()):
    out_path = os.path.join(OUT, rel.replace('/', '_') + '.decomp')
    with open(out_path, 'w') as out:
        out.write(f"# Decompiled reference for: {rel}\n")
        out.write(f"# {len(addrs)} incomplete functions\n\n")
        
        for addr in sorted(addrs):
            processed += 1
            sys.stdout.write(f"\r  [{processed}/{total}] 0x{addr:06X} from {rel}")
            sys.stdout.flush()
            
            decomp = gh_get("/decompile_function", f"address=0x{addr:06X}")
            out.write(f"=== 0x{addr:06X} ===\n")
            out.write(decomp)
            out.write("\n\n")
            time.sleep(0.1)

print(f"\n\nSaved to {OUT}/")
