#!/usr/bin/env python3
"""
Phase 2: Auto-detect and rename sub_ functions via decompilation analysis.
Handles pattern matching for setters, getters, wrappers, and flag-checks.
"""
import json, subprocess, sys, time, re, os
from collections import defaultdict
from urllib.parse import quote

TOKEN = "iwd2ee-re-tools-2026"
BASE = "http://127.0.0.1:8089"
SRC = "/home/wills/projects/IWD2-RE/iwd2-re/src"

def gh_get(path, params=""):
    url = f"{BASE}{path}"
    if params:
        url += "?" + params
    r = subprocess.run(["curl", "-sf", "-m", "30",
        "-H", f"Authorization: Bearer {TOKEN}", url],
        capture_output=True, text=True)
    return r.stdout.strip()

def gh_post(path, data):
    r = subprocess.run(["curl", "-sf", "-X", "POST", "-m", "30",
        "-H", f"Authorization: Bearer {TOKEN}",
        "-H", "Content-Type: application/json",
        "-d", data, f"{BASE}{path}"],
        capture_output=True, text=True)
    return r.stdout.strip()

def decompile(addr_hex):
    return gh_get("/decompile_function", f"address={addr_hex}")

def rename_func(addr_hex, new_name):
    return gh_post("/rename_function_by_address",
        json.dumps({"address": addr_hex, "new_name": new_name}))

# ====== PATTERN DETECTORS ======

def is_simple_setter(code, func_name):
    """Detect: *(param_1 + OFFSET) = param_2; return;"""
    lines = [l.strip() for l in code.split('\n') if l.strip() and not l.strip().startswith('//')]
    if len(lines) < 2:
        return None
    # Look for assignment to *(param_1 + offset)
    m = re.search(r'\*\((?:undefined\d*\s*\*?\s*)?\((?:int|char|byte|short|long|undefined\d*)\)(?:param_\d+|this)\s*\+\s*(0x[0-9a-fA-F]+)\)', code)
    if not m:
        m = re.search(r'\*\((?:undefined\d*\s*\*?\s*)?\(int\)(param_\d+)\s*\+\s*(0x[0-9a-fA-F]+)\)\s*=\s*param_(\d+)', code)
    if not m:
        m = re.search(r'\*(undefined\d*\s*)\*\s*\)?\(param_\d+\s*\+\s*(0x[0-9a-fA-F]+)\)\s*=\s*', code)
    # Simpler: just look for the pattern
    set_match = re.search(r'\+ (0x[0-9A-Fa-f]+)\) =', code)
    if set_match and 'return' in code and len(lines) <= 3:
        return int(set_match.group(1), 16)
    return None

def is_simple_getter(code, func_name):
    """Detect: return *(param_1 + OFFSET);"""
    m = re.search(r'return \*\((?:undefined\d*\s*\*?\s*)?\(int\)(?:param_\d+|this)\s*\+\s*(0x[0-9a-fA-F]+)\)', code)
    if m:
        return int(m.group(1), 16)
    m = re.search(r'return \*\(undefined\d*\s*\*?\s*\)\((?:param_\d+|this)\s*\+\s*(0x[0-9a-fA-F]+)\)', code)
    if m:
        return int(m.group(1), 16)
    return None

def is_boolean_check(code):
    """Detect: return *(param_1 + OFFSET) == value; or comparison"""
    m = re.search(r'return.*(==|!=)\s*(0x[0-9a-fA-F]+|[a-zA-Z_]\w*)', code)
    if m:
        return True
    return False

def is_wrapper(code):
    """Detect if function just calls another function"""
    call_count = code.count('(')
    if call_count <= 2 and 'FUN_' in code:
        return re.findall(r'FUN_([0-9a-fA-F]+)', code)
    return None

def count_lines(code):
    return len([l for l in code.split('\n') if l.strip() and not l.strip().startswith('//') and not l.strip().startswith('{') and not l.strip() == '}'])

# ====== MAIN ======
print("=" * 60)
print("Phase 2: Auto-naming sub_ functions")
print("=" * 60)

with open('/tmp/sub_functions.json') as f:
    sub_funcs = json.load(f)

print(f"Total sub_ functions to process: {len(sub_funcs)}")

# Results tracking
auto_named = []
needs_manual = []

for i, sf in enumerate(sub_funcs):
    addr_hex = sf['addr_hex']
    name = sf['name']
    
    sys.stdout.write(f"\r  [{i+1}/{len(sub_funcs)}] {addr_hex} {name}...")
    sys.stdout.flush()
    
    # Decompile
    decomp = decompile(addr_hex)
    if not decomp:
        needs_manual.append({**sf, 'reason': 'decompilation failed'})
        time.sleep(0.3)
        continue
    
    # Strip function signature header
    lines = decomp.split('\n')
    body_start = 0
    for j, l in enumerate(lines):
        if l.strip() == '{':
            body_start = j
            break
    body = '\n'.join(lines[body_start:]) if body_start > 0 else decomp
    
    class_name = sf.get('class_name', '')
    
    # Try patterns
    offset = is_simple_setter(body, name)
    if offset:
        if class_name:
            # Look up field name at this offset
            field_hint = f"field_{offset:X}"
            new_name = f"Set{field_hint}" if not class_name else f"{class_name}::Set{field_hint}"
        else:
            new_name = f"SetField_{offset:X}"
        auto_named.append({**sf, 'new_name': new_name, 'pattern': 'setter', 'offset': offset})
        continue
    
    offset = is_simple_getter(body, name)
    if offset:
        if class_name:
            field_hint = f"field_{offset:X}"
            new_name = f"Get{field_hint}" if not class_name else f"{class_name}::Get{field_hint}"
        else:
            new_name = f"GetField_{offset:X}"
        auto_named.append({**sf, 'new_name': new_name, 'pattern': 'getter', 'offset': offset})
        continue
    
    if is_boolean_check(body):
        auto_named.append({**sf, 'new_name': None, 'pattern': 'boolean_check', 'needs_naming': True})
        continue
    
    wrapped = is_wrapper(body)
    if wrapped:
        auto_named.append({**sf, 'wraps': wrapped, 'pattern': 'wrapper', 'needs_naming': True})
        continue
    
    line_count = count_lines(decomp)
    if line_count == 0:
        auto_named.append({**sf, 'pattern': 'empty/stub', 'new_name': f'Unimplemented_{sf["addr"]:06X}'})
    else:
        auto_named.append({**sf, 'pattern': 'complex', 'line_count': line_count, 'needs_naming': True})

print(f"\n\nResults:")
print(f"  Auto-named (setter/getter): {sum(1 for a in auto_named if 'new_name' in a and a.get('new_name') and not a.get('needs_naming'))}")
print(f"  Boolean checks:              {sum(1 for a in auto_named if a.get('pattern') == 'boolean_check')}")
print(f"  Wrappers:                   {sum(1 for a in auto_named if a.get('pattern') == 'wrapper')}")
print(f"  Complex (needs manual):     {sum(1 for a in auto_named if a.get('pattern') == 'complex')}")
print(f"  Empty/stub:                 {sum(1 for a in auto_named if a.get('pattern') == 'empty/stub')}")

# Save results
with open('/tmp/phase2_results.json', 'w') as f:
    json.dump({
        'auto_named': auto_named,
        'needs_manual': needs_manual,
        'stats': {
            'total': len(sub_funcs),
            'setter_getter': sum(1 for a in auto_named if a.get('pattern') in ('setter', 'getter')),
            'boolean_checks': sum(1 for a in auto_named if a.get('pattern') == 'boolean_check'),
            'wrappers': sum(1 for a in auto_named if a.get('pattern') == 'wrapper'),
            'complex': sum(1 for a in auto_named if a.get('pattern') == 'complex'),
            'empty': sum(1 for a in auto_named if a.get('pattern') == 'empty/stub'),
        }
    }, f, indent=2)

print(f"\nDetailed results saved to /tmp/phase2_results.json")

# Print some setters/getters as examples
print(f"\n=== Setters/Getters found ===")
for a in auto_named:
    if a.get('pattern') in ('setter', 'getter') and 'new_name' in a:
        print(f"  {a['addr_hex']} {a['name']} -> {a['new_name']} ({a['pattern']} at offset 0x{a['offset']:X})")
        if len([x for x in auto_named if x.get('pattern') in ('setter','getter')]) > 20:
            break

print(f"\n=== Boolean checks (need naming) ===")
for a in auto_named:
    if a.get('pattern') == 'boolean_check':
        print(f"  {a['addr_hex']} {a['name']}  [{a.get('class_name', 'free')}]")

print(f"\n=== Complex (need manual review) ===")
for a in auto_named:
    if a.get('pattern') == 'complex':
        print(f"  {a['addr_hex']} {a['name']} ({a['line_count']} lines) [{a.get('class_name', 'free')}]")
