#!/usr/bin/env python3
"""
Phase 2 Apply v4: Apply manual identifications + decompiled analysis.
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
    print("Phase 2 Apply v4: Manual + auto identifications")
    print("=" * 70)
    
    # Load identified functions
    with open('/tmp/easy_funcs_identified.json') as f:
        funcs = json.load(f)
    
    # Build rename list
    renames = []
    ghidra_renames = []
    
    # Add manual identifications
    for func in funcs:
        if func.get('identified'):
            old = func['name']
            new = func['suggested_name']
            addr = func['addr_hex']
            
            renames.append((old, new))
            ghidra_renames.append((addr, new))
    
    # Also apply previous getters/setters from v3
    with open('/tmp/phase2_applied_v3.json') as f:
        v3_data = json.load(f)
    
    for r in v3_data.get('source_renames', []):
        old = r['old']
        new = r['new']
        if (old, new) not in renames:
            renames.append((old, new))
            ghidra_renames.append((r['addr_hex'], new))
    
    print(f"\nTotal renames to apply: {len(renames)}")
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
    
    print(f"\nDone! Check git diff to verify.")
    print("Run: git diff --stat")
