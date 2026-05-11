#!/usr/bin/env python3
"""
Phase 2 Apply v5: Apply all identifications including wrappers.
"""
import json, re, os, subprocess

SRC = "/home/wills/projects/IWD2-RE/iwd2-re/src"
SOCKET = "/run/user/1000/ghidra-mcp/ghidra-1390827.sock"

def gh_rename(addr_hex, new_name):
    url = f"http://localhost/rename_function_by_address"
    data = json.dumps({"address": addr_hex, "new_name": new_name})
    r = subprocess.run(
        ["curl", "-sf", "-X", "POST", "-m", "30", "--unix-socket", SOCKET,
         "-H", "Content-Type: application/json", "-d", data, url],
        capture_output=True, text=True
    )
    return r.stdout.strip()

def apply_to_source(renames):
    files_changed = 0
    total = 0
    
    for root, dirs, files in os.walk(SRC):
        for file in files:
            if file.endswith(('.cpp', '.h')):
                filepath = os.path.join(root, file)
                with open(filepath, 'r') as f:
                    content = f.read()
                
                original = content
                for old, new in renames:
                    content = re.sub(rf'\b{re.escape(old)}\b', new, content)
                
                if content != original:
                    with open(filepath, 'w') as f:
                        f.write(content)
                    files_changed += 1
                    total += sum(1 for old, new in renames if old in original)
    
    return files_changed, total

def apply_to_ghidra(renames):
    success = 0
    failed = 0
    
    for addr_hex, new_name in renames:
        result = gh_rename(addr_hex, new_name)
        if result:
            success += 1
            print(f"  ✓ {addr_hex} -> {new_name}")
        else:
            failed += 1
            print(f"  ✗ {addr_hex} -> {new_name} (failed)")
    
    return success, failed

if __name__ == '__main__':
    print("=" * 70)
    print("Phase 2 Apply v5: Wrappers + previous identifications")
    print("=" * 70)
    
    renames = []
    ghidra_renames = []
    
    # Load all identifications
    # 1. From v3 (getters/setters)
    with open('/tmp/phase2_applied_v3.json') as f:
        v3 = json.load(f)
    for r in v3.get('source_renames', []):
        renames.append((r['old'], r['new']))
        ghidra_renames.append((r['addr_hex'], r['new']))
    
    # 2. From manual analysis v4
    with open('/tmp/easy_funcs_identified.json') as f:
        funcs = json.load(f)
    
    for func in funcs:
        if func.get('identified') and func.get('suggested_name'):
            old = func['name']
            new = func['suggested_name']
            addr = func['addr_hex']
            
            if (old, new) not in renames:
                renames.append((old, new))
                ghidra_renames.append((addr, new))
    
    print(f"\nTotal unique renames: {len(renames)}")
    for old, new in renames:
        print(f"  {old} -> {new}")
    
    # Apply to source
    print("\nApplying to source...")
    files_changed, total = apply_to_source(renames)
    print(f"  Changed {files_changed} files, {total} replacements")
    
    # Apply to Ghidra
    print("\nApplying to Ghidra...")
    success, failed = apply_to_ghidra(ghidra_renames)
    print(f"  Success: {success}, Failed: {failed}")
    
    print("\nDone! Run 'git diff --stat' to see all changes.")
