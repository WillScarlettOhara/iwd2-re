#!/usr/bin/env python3
"""
Phase 5: Batch rename field_X members with safety checks.

Rules:
- Only rename within files explicitly listed
- Skip fields that appear in multiple classes in the same file
- Generate a report of changes made
"""
import sys, os, re
from collections import defaultdict

renames = [
    # (old_name, new_name, header_file, impl_files)
    {
        'old': 'nm_field_118',
        'new': 'm_bCopyError',
        'class': 'CResCache',
        'header': 'CResCache.h',
        'impls': ['CResCache.cpp'],
        'confidence': 'High',
    },
    {
        'old': 'nm_field_110',
        'new': 'm_bCacheLocked',
        'class': 'CResCache',
        'header': 'CResCache.h',
        'impls': ['CResCache.cpp', 'CResFile.cpp', 'CDimm.cpp'],
        'confidence': 'High',
    },
    {
        'old': 'sm_field_10C',
        'new': 'm_sTempFileName',
        'class': 'CResCache',
        'header': 'CResCache.h',
        'impls': [],  # Unused
        'confidence': 'Low',
    },
]

SRC = "/home/wills/projects/IWD2-RE/iwd2-re/src"

def safe_rename(old, new, cls, header, impls, dry_run=True):
    files = [header] + impls
    changes = []
    
    for fname in files:
        fpath = os.path.join(SRC, fname)
        if not os.path.exists(fpath):
            print(f"SKIP: {fpath} not found")
            continue
        
        with open(fpath, 'r') as f:
            content = f.read()
        
        # Check if there are OTHER classes with the same field name in this file
        # Simple heuristic: if the field name appears and it's not qualified by our class, skip
        other_occurrences = []
        for m in re.finditer(r'\b' + re.escape(old) + r'\b', content):
            # Check context - is this a comment?
            line_start = content.rfind('\n', 0, m.start()) + 1
            line_end = content.find('\n', m.end())
            line = content[line_start:line_end].strip()
            if '//' in line and line.index('//') < m.start() - line_start:
                continue  # It's in a comment
            
            # Check if line has a class qualifier different from our target
            # This is a heuristic
            if '.' in line or '->' in line:
                if 'cResCache.' not in line and 'cDimm.cResCache.' not in line and 'pResCache->' not in line and 'cDimm.pResCache->' not in line:
                    other_occurrences.append(line.strip())
        
        if other_occurrences:
            print(f"WARNING: {fname} has ambiguous {old} usage:")
            for occ in other_occurrences:
                print(f"  {occ}")
            print(f"  Skipping {fname}")
            continue
        
        count = content.count(old)
        if count == 0:
            continue
        
        if not dry_run:
            new_content = content.replace(old, new)
            with open(fpath, 'w') as f:
                f.write(new_content)
        
        changes.append(f"  {fname}: {count} occurrences")
    
    return changes

def main():
    dry_run = '--apply' not in sys.argv
    
    print(f"Phase 5 Batch Rename (dry_run={dry_run})")
    print("=" * 50)
    
    total = 0
    for cfg in renames:
        print(f"\n{cfg['class']}::{cfg['old']} -> {cfg['new']} ({cfg['confidence']})")
        changes = safe_rename(
            cfg['old'], cfg['new'], cfg['class'],
            cfg['header'], cfg['impls'], dry_run=dry_run
        )
        if changes:
            for c in changes:
                print(c)
                total += 1
        else:
            print("  No changes (skipped or not found)")
    
    print(f"\n{'='*50}")
    print(f"Total files to change: {total}")
    if dry_run:
        print("Run with --apply to execute.")

if __name__ == '__main__':
    main()
