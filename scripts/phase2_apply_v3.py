#!/usr/bin/env python3
"""
Phase 2 Apply v3: Clean renames for getters/setters with proper PascalCase.
Handles field_X fields correctly.
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
    """Convert field name to PascalCase method suffix"""
    clean = field_name
    
    # Handle field_XXX format
    if clean.startswith('field_'):
        # Keep the hex number, prefix with "Field" for readability
        suffix = clean[6:]  # After "field_"
        return f"Field_{suffix}"
    
    # Remove common Hungarian prefixes
    prefixes = ['m_', 'n', 'b', 'w', 'dw', 'pm_', 'p', 's', 'l', 'h', 'by', 'f', 'c', 'u']
    
    for prefix in prefixes:
        if clean.startswith(prefix):
            clean = clean[len(prefix):]
            break
    
    # Convert snake_case to PascalCase
    words = clean.split('_')
    result = ''.join(word[0].upper() + word[1:] if len(word) > 1 else word.upper() for word in words if word)
    
    return result if result else field_name

def apply_renames():
    # Known field mappings from header analysis
    # Will be loaded from source dynamically
    known_fields = {}
    
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
        
        # Try to find field name in source
        field_name = find_field_name(class_name, offset)
        if not field_name:
            field_name = f'field_{offset:X}'
        
        # Clean for method name
        clean = clean_field_name(field_name)
        
        # Build new name
        action = 'Get' if pattern == 'getter' else 'Set'
        if class_name:
            short_name = f"{action}{clean}"
            new_name = f"{class_name}::{action}{clean}"
        else:
            short_name = f"{action}{clean}"
            new_name = short_name
        
        renames.append({
            'old': old_name,
            'new': short_name,
            'full': new_name,
            'class': class_name,
            'addr_hex': addr_hex,
            'pattern': pattern,
            'offset': offset,
            'field_name': field_name
        })
        
        ghidra_renames.append({
            'addr_hex': addr_hex,
            'new_name': short_name
        })
    
    return renames, ghidra_renames

def find_field_name(class_name, offset):
    """Find field name at given offset in class headers"""
    if not class_name:
        return None
    
    offset_hex = f"{offset:04X}"
    offset_pattern = f"/* {offset_hex} */"
    
    for root, dirs, files in os.walk(SRC):
        for file in files:
            if file.endswith('.h'):
                filepath = os.path.join(root, file)
                with open(filepath, 'r') as f:
                    content = f.read()
                    
                if class_name in content:
                    lines = content.split('\n')
                    for line in lines:
                        if offset_pattern in line:
                            # Extract the field name (last word before ;)
                            parts = line.strip().rstrip(';').split()
                            if parts:
                                return parts[-1]
    
    return None

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
                    
                    # Replace whole word occurrences
                    content = re.sub(rf'\b{re.escape(old)}\b', new, content)
                
                if content != original:
                    with open(filepath, 'w') as f:
                        f.write(content)
                    files_changed += 1
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
    print("Phase 2 Apply v3: Applying clean renames (field_X fixed)")
    print("=" * 70)
    
    renames, ghidra_renames = apply_renames()
    
    print(f"\nRenames to apply: {len(renames)}")
    for r in renames:
        print(f"  {r['old']} -> {r['new']} (field={r['field_name']}) [{r['class']}]")
    
    # Apply to source
    print("\nApplying to source files...")
    files_changed, total = apply_to_source(renames)
    print(f"  Changed {files_changed} files, {total} total replacements")
    
    # Apply to Ghidra
    print("\nApplying to Ghidra binary...")
    success, failed = apply_to_ghidra(ghidra_renames)
    print(f"  Success: {success}, Failed: {failed}")
    
    # Save mapping
    with open('/tmp/phase2_applied_v3.json', 'w') as f:
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
    
    print(f"\nSaved to /tmp/phase2_applied_v3.json")
    print("\nDone! Check git diff to verify changes.")
