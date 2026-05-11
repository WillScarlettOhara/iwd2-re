#!/usr/bin/env python3
"""
Phase 2 Apply: Apply auto-detected names to source files.
Renames sub_ functions based on pattern analysis.
"""
import json, re, os, glob

SRC = "/home/wills/projects/IWD2-RE/iwd2-re/src"

def apply_renames():
    with open('/tmp/phase2_results_v2.json') as f:
        data = json.load(f)
    
    renames = []  # (old_pattern, new_name, reason)
    
    for item in data.get('auto_named', []):
        addr = item['addr']
        addr_hex = item['addr_hex']
        old_name = item['name']
        class_name = item.get('class_name', '')
        
        if item.get('pattern') == 'getter' and 'new_name' in item:
            # Getter: Try to find the actual field name from source
            offset = item.get('offset', 0)
            field_name = find_field_name(class_name, offset)
            if field_name:
                if class_name:
                    new_name = f"{class_name}::Get{field_name}"
                else:
                    new_name = f"Get{field_name}"
            else:
                new_name = item['new_name']  # Use auto-generated
            renames.append((old_name, new_name, 'getter'))
            
        elif item.get('pattern') == 'setter' and 'new_name' in item:
            offset = item.get('offset', 0)
            field_name = find_field_name(class_name, offset)
            if field_name:
                if class_name:
                    new_name = f"{class_name}::Set{field_name}"
                else:
                    new_name = f"Set{field_name}"
            else:
                new_name = item['new_name']
            renames.append((old_name, new_name, 'setter'))
            
        elif item.get('pattern') == 'wrapper':
            # Wrapper: try to guess based on what it wraps
            wraps = item.get('wraps', [])
            if wraps:
                wrapped_addr = wraps[0]
                # If it wraps a destructor pattern (delete, ~Class)
                new_name = guess_wrapper_name(class_name, wrapped_addr, item)
                if new_name:
                    renames.append((old_name, new_name, 'wrapper'))
    
    return renames

def find_field_name(class_name, offset):
    """Try to find field name at given offset in class headers"""
    if not class_name:
        return None
    
    # Search in header files
    pattern = f"/* {offset:04X} */"
    for root, dirs, files in os.walk(SRC):
        for file in files:
            if file.endswith('.h'):
                filepath = os.path.join(root, file)
                with open(filepath, 'r') as f:
                    content = f.read()
                    if class_name in content:
                        # Found file with class, search for offset
                        lines = content.split('\n')
                        for line in lines:
                            if pattern in line and ('field_' in line or 'm_' in line or 'n' in line or 'b' in line):
                                # Extract field name
                                parts = line.split()
                                for i, part in enumerate(parts):
                                    if part.startswith('field_') or part.startswith('m_') or part.startswith('n') or part.startswith('b'):
                                        # Remove semicolon
                                        name = part.rstrip(';')
                                        return name
    return None

def guess_wrapper_name(class_name, wrapped_addr, item):
    """Guess wrapper function name based on context"""
    # Common patterns:
    # - Wrapping own address: often destructor or simple redirect
    # - Wrapping parent class method: inherited method
    
    addr = item.get('addr', 0)
    if wrapped_addr.lower() == f"{addr:06x}".lower():
        # Self-wrap or recursive - might be destructor
        if class_name:
            return f"{class_name}::~{class_name.split('::')[-1]}"
    
    # Check if it's a known operator or common method
    return None

def apply_to_source(renames):
    """Apply renames to all source files"""
    files_changed = 0
    total_renames = 0
    
    for root, dirs, files in os.walk(SRC):
        for file in files:
            if file.endswith(('.cpp', '.h')):
                filepath = os.path.join(root, file)
                with open(filepath, 'r') as f:
                    content = f.read()
                
                original = content
                for old_name, new_name, reason in renames:
                    # Replace function definitions and declarations
                    # Pattern: return_type old_name( -> return_type new_name(
                    content = re.sub(
                        rf'\b{re.escape(old_name)}\b',
                        new_name.split('::')[-1] if '::' in new_name else new_name,
                        content
                    )
                    
                    # Also replace in comments
                    content = content.replace(f'// 0x{old_name[4:]}', f'// 0x{old_name[4:]}')
                
                if content != original:
                    with open(filepath, 'w') as f:
                        f.write(content)
                    files_changed += 1
                    total_renames += sum(1 for old, new, _ in renames if old in original)
    
    return files_changed, total_renames

if __name__ == '__main__':
    print("=" * 70)
    print("Phase 2 Apply: Applying auto-detected names to source")
    print("=" * 70)
    
    renames = apply_renames()
    print(f"\nRenames to apply: {len(renames)}")
    for old, new, reason in renames[:20]:
        print(f"  {old} -> {new} ({reason})")
    if len(renames) > 20:
        print(f"  ... and {len(renames) - 20} more")
    
    print("\nApply to source? (y/n)")
    # Auto-apply for now
    files_changed, total_renames = apply_to_source(renames)
    print(f"\nApplied {total_renames} renames across {files_changed} files")
    
    # Save mapping for reference
    with open('/tmp/phase2_applied_renames.json', 'w') as f:
        json.dump([{'old': o, 'new': n, 'reason': r} for o, n, r in renames], f, indent=2)
    
    print("Saved rename mapping to /tmp/phase2_applied_renames.json")
