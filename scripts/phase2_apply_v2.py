#!/usr/bin/env python3
"""
Phase 2 Apply v2: Clean renames for getters/setters with proper PascalCase.
"""
import json, re, os, subprocess

SRC = "/home/wills/projects/IWD2-RE/iwd2-re/src"
SOCKET = "/run/user/1000/ghidra-mcp/ghidra-1390827.sock"

def gh_rename(addr_hex, new_name):
    """Rename function in Ghidra via MCP"""
    url = f"http://localhost/rename_function_by_address"
    data = json.dumps({"address": addr_hex, "new_name": new_name})
    r = subprocess.run(
        ["curl", "-sf", "-X", "POST", "-m", "30", "--unix-socket", SOCKET,
         "-H", "Content-Type: application/json", "-d", data, url],
        capture_output=True, text=True
    )
    return r.stdout.strip()

def clean_field_name(field_name):
    """Convert Hungarian notation field to PascalCase method suffix"""
    # Remove common prefixes
    prefixes = ['m_', 'n', 'b', 'w', 'dw', 'pm_', 'p', 's', 'l', 'h', 'by', 'f', 'c', 'u']
    clean = field_name
    
    for prefix in prefixes:
        if clean.startswith(prefix):
            clean = clean[len(prefix):]
            break
    
    # Handle bm_, field_ etc
    if clean.startswith('field_'):
        clean = clean[6:]
    if clean.startswith('m_'):
        clean = clean[2:]
    
    # Convert to PascalCase (capitalize first letter of each word)
    # Split by underscore and capitalize
    words = clean.split('_')
    result = ''.join(word[0].upper() + word[1:] if word else '' for word in words)
    
    return result if result else field_name

def apply_renames():
    # Known field mappings from header analysis
    known_fields = {
        ('CGameSprite', 0x7240): 'm_bHiding',
        ('CGameDoor', 0x5C4): 'm_dwFlags',
        ('CGameAreaNotes', 0x8D): 'bm_field_8D',
        ('CGameAIBase', 0x58C): 'nfield_58C',
    }
    
    with open('/tmp/phase2_results_v2.json') as f:
        data = json.load(f)
    
    renames = []
    ghidra_renames = []
    
    for item in data.get('auto_named', []):
        if item.get('pattern') not in ('getter', 'setter'):
            continue
            
        addr_hex = item['addr_hex']
        old_name = item['name']
        class_name = item.get('class_name', '')
        offset = item.get('offset', 0)
        pattern = item['pattern']
        
        # Get field name
        field_key = (class_name, offset)
        field_name = known_fields.get(field_key, f'field_{offset:X}')
        
        # Clean for method name
        clean = clean_field_name(field_name)
        
        # Build new name
        action = 'Get' if pattern == 'getter' else 'Set'
        if class_name:
            new_name = f"{class_name}::{action}{clean}"
            short_name = f"{action}{clean}"
        else:
            new_name = f"{action}{clean}"
            short_name = new_name
        
        renames.append({
            'old': old_name,
            'new': short_name,
            'full': new_name,
            'class': class_name,
            'addr_hex': addr_hex,
            'pattern': pattern,
            'offset': offset
        })
        
        ghidra_renames.append({
            'addr_hex': addr_hex,
            'new_name': short_name
        })
    
    return renames, ghidra_renames

def apply_to_source(renames):
    """Apply renames to source files"""
    files_changed = 0
    total_replacements = 0
    
    for root, dirs, files in os.walk(SRC):
        for file in files:
            if file.endswith(('.cpp', '.h')):
                filepath = os.path.join(root, file)
                with open(filepath, 'r') as f:
                    content = f.read()
                
                original = content
                for r in renames:
                    old = r['old']
                    new = r['new']
                    
                    # Replace function declarations/definitions
                    # Must be whole word to avoid partial matches
                    content = re.sub(rf'\b{re.escape(old)}\b', new, content)
                
                if content != original:
                    with open(filepath, 'w') as f:
                        f.write(content)
                    files_changed += 1
                    # Count how many unique renames were in this file
                    file_renames = sum(1 for r in renames if r['old'] in original)
                    total_replacements += file_renames
    
    return files_changed, total_replacements

def apply_to_ghidra(ghidra_renames):
    """Apply renames to Ghidra binary"""
    success = 0
    failed = 0
    
    for r in ghidra_renames:
        addr = r['addr_hex']
        name = r['new_name']
        
        result = gh_rename(addr, name)
        if result:
            success += 1
            print(f"  ✓ {addr} -> {name}")
        else:
            failed += 1
            print(f"  ✗ {addr} -> {name} (failed)")
    
    return success, failed

if __name__ == '__main__':
    print("=" * 70)
    print("Phase 2 Apply v2: Applying clean renames")
    print("=" * 70)
    
    renames, ghidra_renames = apply_renames()
    
    print(f"\nRenames to apply: {len(renames)}")
    for r in renames:
        print(f"  {r['old']} -> {r['new']} ({r['pattern']}) [{r['class']}]")
    
    # Apply to source
    print("\nApplying to source files...")
    files_changed, total = apply_to_source(renames)
    print(f"  Changed {files_changed} files, {total} total replacements")
    
    # Apply to Ghidra
    print("\nApplying to Ghidra binary...")
    success, failed = apply_to_ghidra(ghidra_renames)
    print(f"  Success: {success}, Failed: {failed}")
    
    # Save mapping
    with open('/tmp/phase2_applied_v2.json', 'w') as f:
        json.dump({
            'source_renames': renames,
            'ghidra_renames': ghidra_renames,
            'stats': {
                'total': len(renames),
                'files_changed': files_changed,
                'replacements': total,
                'ghidra_success': success,
                'ghidra_failed': failed
            }
        }, f, indent=2)
    
    print(f"\nSaved to /tmp/phase2_applied_v2.json")
    print("\nDone! Check git diff to see changes.")
