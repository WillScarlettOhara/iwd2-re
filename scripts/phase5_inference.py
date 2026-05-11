#!/usr/bin/env python3
"""
Phase 5: Field Name Inference Script

Strategy: Find functions with real names (not sub_ or GetField_xxx) 
that access field_X members. The function name is a strong hint
for what the field represents.

Outputs a JSON mapping: (class, field_name) -> suggested_name
with confidence scores based on usage patterns.
"""
import json, re, os
from collections import Counter, defaultdict

SRC_DIR = "/home/wills/projects/IWD2-RE/iwd2-re/src"

def find_class_fields():
    """Build map: (class_name, field_name) -> list of (type, offset_comment)"""
    class_fields = defaultdict(list)
    
    for root, dirs, files in os.walk(SRC_DIR):
        for file in files:
            if not file.endswith('.h'):
                continue
            filepath = os.path.join(root, file)
            with open(filepath, 'r') as f:
                content = f.read()
            
            # Extract class/struct name
            class_name = None
            for m in re.finditer(r'(class|struct)\s+(\w+)', content):
                class_name = m.group(2)
            
            if not class_name:
                continue
            
            # Find field declarations with offset comments
            for m in re.finditer(r'(/\*\s*(0x[0-9A-Fa-f]+|[0-9A-Fa-f]+)\s*\*/)?\s*(.*?)\b([a-zA-Z_]*)field_([0-9A-Fa-f_]+)\s*;', content):
                offset = m.group(2) if m.group(2) else ''
                decl_type = m.group(3).strip() if m.group(3) else ''
                prefix = m.group(4) if m.group(4) else ''
                offset_num = m.group(5)
                field_name = f"{prefix}field_{offset_num}"
                class_fields[class_name].append((field_name, offset, decl_type, prefix, offset_num))
    
    return class_fields

def find_function_field_accesses():
    """Find functions (named, not sub_) that access field_X members.
    Returns list of (class_name, func_name, field_name, context_line)"""
    results = []
    
    for root, dirs, files in os.walk(SRC_DIR):
        for file in files:
            if not file.endswith('.cpp'):
                continue
            filepath = os.path.join(root, file)
            with open(filepath, 'r') as f:
                content = f.read()
            
            # Parse function bodies. Regex-based heuristic:
            # Find function definitions
            for m in re.finditer(
                r'(?P<decl>(?:[\w:]+\s+))?(?P<name>\w+::[A-Za-z][A-Za-z0-9_]+)\s*\([^)]*\)\s*\{',
                content
            ):
                func_name = m.group('name')
                
                # Skip if function name contains sub_, GetField_, SetField_
                if re.search(r'(sub_|GetField_|SetField_|HasField_|IsField_)', func_name):
                    continue
                
                # Find the function body (simple brace matching)
                start = m.end()
                brace_count = 1
                end = start
                while end < len(content) and brace_count > 0:
                    if content[end] == '{':
                        brace_count += 1
                    elif content[end] == '}':
                        brace_count -= 1
                    end += 1
                
                body = content[start:end]
                
                # Find field accesses in body
                for fm in re.finditer(r'\b([a-zA-Z_]*)field_([0-9A-Fa-f_]+)\b', body):
                    prefix = fm.group(1)
                    offset = fm.group(2)
                    field_name = f"{prefix}field_{offset}"
                    
                    # Get surrounding line
                    line_start = body.rfind('\n', 0, fm.start())
                    line_end = body.find('\n', fm.end())
                    line = body[line_start:line_end].strip()
                    
                    results.append((func_name, field_name, line))
    
    return results

def infer_names_from_getters_setters():
    """Use existing getter/setter names to infer field names."""
    inferred = {}
    
    # From previous phases and known mappings
    known = {
        'CGameSprite': {
            'm_field_7240': 'm_bHiding',
            'm_field_5304': 'm_nActionTimer',  # guessed from context
        },
        'CGameAIBase': {
            'm_field_58C': 'm_nAITarget',
        },
        'CGameDoor': {
            'm_field_5C4': 'm_dwFlags',
        },
    }
    
    return known

def main():
    print("=== Finding class field declarations ===")
    class_fields = find_class_fields()
    print(f"Found fields in {len(class_fields)} classes/structs")
    
    # Show top classes
    top = sorted(class_fields.items(), key=lambda x: -len(x[1]))[:10]
    for cls, fields in top:
        print(f"  {cls}: {len(fields)} fields")
        for f, off, t, p, num in fields[:3]:
            print(f"    {off} {t} {f}")
    
    print("\n=== Finding function -> field access ===")
    accesses = find_function_field_accesses()
    print(f"Found {len(accesses)} function->field accesses")
    
    # Group by field name
    field_funcs = defaultdict(list)
    for func, field, line in accesses:
        field_funcs[field].append(func)
    
    # Show fields with named functions accessing them
    for field, funcs in sorted(field_funcs.items(), key=lambda x: -len(x[1]))[:20]:
        print(f"  {field:25s} accessed by {len(funcs)} named functions")
        for f in funcs[:3]:
            print(f"    - {f}")

if __name__ == '__main__':
    main()
