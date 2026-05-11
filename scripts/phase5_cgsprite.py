#!/usr/bin/env python3
"""
Phase 5: Targeted CGameSprite field inference.

For each m_field_XXXX in CGameSprite.h:
1. Find all named functions in CGameSprite.cpp that read/write it
2. Collect functional context (which methods touch it)
3. Use decompiler/MCP to inspect actual value usage
4. Suggest semantic names
"""
import json, re, os
from collections import defaultdict

SRC = "/home/wills/projects/IWD2-RE/iwd2-re/src"

def extract_cgsprite_fields():
    """Get all m_field_ members from CGameSprite.h with offsets"""
    fields = []
    with open(os.path.join(SRC, 'CGameSprite.h'), 'r') as f:
        content = f.read()
    
    for m in re.finditer(r'(/\*\s*(0x[0-9A-Fa-f]+|[0-9A-Fa-f]+)\s*\*/)?\s*(.*?)\b(m_field_[0-9A-Fa-f_]+)\b;', content):
        offset = m.group(2) if m.group(2) else ''
        dtype = m.group(3).strip() if m.group(3) else ''
        fname = m.group(4)
        fields.append({'fname': fname, 'offset': offset, 'type': dtype})
    
    return fields

def find_accessors(field_name, cpp_file='CGameSprite.cpp'):
    """Find all functions in cpp_file that access field_name."""
    filepath = os.path.join(SRC, cpp_file)
    with open(filepath, 'r') as f:
        content = f.read()
    
    funcs = []
    
    # Find all function definitions and their bodies
    for m in re.finditer(
        r'(?P<decl>(?:[\w:\*\s&]+\s+))?(?P<func>[A-Za-z][A-Za-z0-9_:]*\s*\([^)]*\))\s*(?P<const>const)?\s*\{',
        content
    ):
        func_sig = m.group('func').strip()
        func_start = m.end()
        
        # Extract function name
        func_name = func_sig.split('(')[0].strip()
        
        # Skip sub_ and field accessors
        if re.search(r'sub_|GetField_|SetField_|HasField_|IsField_', func_name):
            continue
        
        # Find function body
        brace_count = 1
        pos = func_start
        while pos < len(content) and brace_count > 0:
            if content[pos] == '{':
                brace_count += 1
            elif content[pos] == '}':
                brace_count -= 1
            pos += 1
        
        body = content[func_start:pos]
        
        # Check if field_name appears in body
        if field_name in body:
            # Count occurrences
            count = body.count(field_name)
            # Get a few lines of context
            lines = []
            for fm in re.finditer(r'\b' + re.escape(field_name) + r'\b', body):
                line_start = body.rfind('\n', 0, fm.start()) + 1
                line_end = body.find('\n', fm.end())
                line = body[line_start:line_end].strip()
                lines.append(line)
            funcs.append({'name': func_name, 'count': count, 'lines': lines[:3]})
    
    return funcs

def main():
    print("=== Phase 5: CGameSprite m_field_ Analysis ===\n")
    
    fields = extract_cgsprite_fields()
    print(f"Found {len(fields)} m_field_ members in CGameSprite\n")
    
    # Sort by offset
    # Extract hex offset for sorting
    def get_offset_hex(f):
        off = f['offset']
        if off.startswith('0x'):
            return int(off, 16)
        else:
            try:
                return int(off, 16)
            except:
                return 0
    
    fields_sorted = sorted(fields, key=get_offset_hex)
    
    # For each field, show accessor context
    for field in fields_sorted:
        print(f"---\nField: {field['fname']}  offset: {field['offset']}  type: {field['type']}")
        accessors = find_accessors(field['fname'])
        if accessors:
            print(f"  Accessed by {len(accessors)} named functions")
            for acc in accessors[:5]:
                print(f"    {acc['name']} ({acc['count']} refs)")
                for line in acc['lines']:
                    print(f"      {line[:80]}")
        else:
            print(f"  Only accessed by sub_/unidentified functions")

if __name__ == '__main__':
    main()
