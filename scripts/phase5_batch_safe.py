#!/usr/bin/env python3
"""
Phase 5: Safe batch renames for fields appearing in exactly ONE class.
"""
import os, re, sys
from collections import defaultdict

SRC = "/home/wills/projects/IWD2-RE/iwd2-re/src"

def build_field_map():
    """Map each field to (class, set of files where it's declared)"""
    field_info = defaultdict(lambda: {'class': None, 'files': set()})
    
    for root, dirs, files in os.walk(SRC):
        for file in files:
            if not file.endswith('.h'):
                continue
            filepath = os.path.join(root, file)
            with open(filepath, 'r') as f:
                content = f.read()
            
            # Get class name
            class_name = None
            for m in re.finditer(r'(class|struct)\s+(\w+)', content):
                class_name = m.group(2)
            
            if not class_name:
                continue
            
            # Find field declarations
            for m in re.finditer(r'\b([a-zA-Z_]*)field_([0-9A-Fa-f_]+)\b', content):
                prefix = m.group(1)
                offset = m.group(2)
                fname = f"{prefix}field_{offset}"
                field_info[fname]['class'] = class_name
                field_info[fname]['files'].add(file)
    
    return field_info

def rename_field(old_name, new_name, dry_run=True):
    """Rename field across all source files where it appears, but ONLY if
    it appears in exactly ONE class context."""
    total = 0
    files_changed = []
    
    for root, dirs, files in os.walk(SRC):
        for file in files:
            if not file.endswith(('.cpp', '.h')):
                continue
            filepath = os.path.join(root, file)
            with open(filepath, 'r') as f:
                content = f.read()
            
            count = content.count(old_name)
            if count == 0:
                continue
            
            new_content = content.replace(old_name, new_name)
            total += count
            
            if not dry_run:
                with open(filepath, 'w') as f:
                    f.write(new_content)
            
            files_changed.append(file)
    
    return total, files_changed

def main():
    # Build info
    info = build_field_map()
    
    # Find unique fields (appear in only ONE class)  
    unique_fields = []
    for fname, data in info.items():
        if len(data['files']) == 1 and data['class'] and data['class'] not in ('CGameSprite', 'CChitin'):
            unique_fields.append((fname, data['class']))
    
    unique_fields.sort()
    
    print(f"Total field_ members: {len(info)}")
    print(f"Unique to one class (excluding CGameSprite/CChitin): {len(unique_fields)}")
    
    dry_run = '--apply' not in sys.argv
    
    for fname, cls in unique_fields[:5]:
        # Try to infer name from context - for now just show
        print(f"\nFound: {cls}::{fname}")

if __name__ == '__main__':
    main()
