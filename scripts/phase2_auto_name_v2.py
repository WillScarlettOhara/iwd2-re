#!/usr/bin/env python3
"""
Phase 2: Auto-detect and rename sub_ functions via decompilation analysis.
Handles pattern matching for setters, getters, wrappers, and flag-checks.
"""
import json, subprocess, sys, time, re, os
from collections import defaultdict
from urllib.parse import quote

# MCP via Unix socket (Ghidra GUI instance)
SOCKET = "/run/user/1000/ghidra-mcp/ghidra-1390827.sock"
SRC = "/home/wills/projects/IWD2-RE/iwd2-re/src"

def gh_get(path, params=""):
    """Call MCP via Unix socket using curl"""
    url = f"http://localhost{path}"
    if params:
        url += "?" + params
    r = subprocess.run(
        ["curl", "-sf", "-m", "60", "--unix-socket", SOCKET, url],
        capture_output=True, text=True
    )
    return r.stdout.strip()

def decompile(addr_hex):
    return gh_get("/decompile_function", f"address={addr_hex}")

def analyze_function(addr_hex):
    """Get full function analysis with decompilation"""
    return gh_get("/analyze_function_complete", f"address={addr_hex}&include_disasm=false&include_variables=true")

# ====== PATTERN DETECTORS ======

def is_simple_setter(code, func_name):
    """Detect: *(param_1 + OFFSET) = param_2; return;"""
    # Look for assignment pattern: *(type*)(param + offset) = value
    m = re.search(r'\*\([^)]*\)\s*\(\s*(?:param_\d+|this)\s*\+\s*(0x[0-9a-fA-F]+)\)\s*=\s*', code)
    if m:
        offset = int(m.group(1), 16)
        # Check if it's a simple setter (few lines, mostly assignment)
        lines = [l for l in code.split('\n') if l.strip() and not l.strip().startswith('//') and l.strip() not in ['{', '}']]
        if len(lines) <= 3:
            return offset
    return None

def is_simple_getter(code, func_name):
    """Detect: return *(param_1 + OFFSET);"""
    # Multiple patterns for Ghidra decompiler output
    patterns = [
        r'return\s+\*\([^)]*\)\s*\(\s*(?:param_\d+|this)\s*\+\s*(0x[0-9a-fA-F]+)\)',
        r'return\s+\*\([^)]*\+\s*(0x[0-9a-fA-F]+)\)',
        r'return\s+\*\s*\([^)]*\+\s*(0x[0-9a-fA-F]+)\)',
    ]
    for pattern in patterns:
        m = re.search(pattern, code)
        if m:
            offset = int(m.group(1), 16)
            # Verify it's simple (just a return)
            lines = [l for l in code.split('\n') if l.strip() and not l.strip().startswith('//') and l.strip() not in ['{', '}']]
            if len(lines) <= 3:
                return offset
    return None

def is_boolean_check(code):
    """Detect: return *(param_1 + OFFSET) == value; or comparison"""
    m = re.search(r'return.*(==|!=|<=|>=|<|>)\s*(0x[0-9a-fA-F]+|[a-zA-Z_]\w*)', code)
    if m:
        return True
    return False

def is_wrapper(code):
    """Detect if function just calls another function with same args"""
    # Count actual function calls (FUN_ or known names)
    fun_calls = re.findall(r'FUN_([0-9a-fA-F]+)', code)
    if len(fun_calls) == 1:
        # Single call, check if it returns directly
        if 'return' in code or 'RET' in code:
            return fun_calls
    return None

def count_lines(code):
    """Count meaningful lines"""
    return len([l for l in code.split('\n') 
                if l.strip() and not l.strip().startswith('//') 
                and l.strip() not in ['{', '}']])

# ====== MAIN ======
print("=" * 70)
print("Phase 2: Auto-naming sub_ functions via MCP decompilation")
print("=" * 70)

with open('/tmp/sub_functions.json') as f:
    sub_funcs = json.load(f)

print(f"Total sub_ functions to process: {len(sub_funcs)}")
print()

# Results tracking
auto_named = []
needs_manual = []

for i, sf in enumerate(sub_funcs):
    addr_hex = sf['addr_hex']
    name = sf['name']
    
    sys.stdout.write(f"\r  [{i+1}/{len(sub_funcs)}] {addr_hex} {name}...{' ' * 30}")
    sys.stdout.flush()
    
    # Get decompilation via MCP
    try:
        decomp = decompile(addr_hex)
    except Exception as e:
        needs_manual.append({**sf, 'reason': f'exception: {e}'})
        time.sleep(0.1)
        continue
    
    if not decomp:
        # Try analyze_function_complete instead
        try:
            analysis = analyze_function(addr_hex)
            if analysis:
                # Parse JSON to get decompiled_code
                import json
                data = json.loads(analysis)
                decomp = data.get('decompiled_code', '')
        except:
            pass
        
        if not decomp:
            needs_manual.append({**sf, 'reason': 'decompilation failed'})
            time.sleep(0.1)
            continue
    
    class_name = sf.get('class_name', '')
    
    # Try patterns on the decompiled code
    offset = is_simple_setter(decomp, name)
    if offset is not None:
        if class_name:
            new_name = f"{class_name}::SetField_{offset:X}"
        else:
            new_name = f"SetField_{offset:X}"
        auto_named.append({**sf, 'new_name': new_name, 'pattern': 'setter', 'offset': offset})
        continue
    
    offset = is_simple_getter(decomp, name)
    if offset is not None:
        if class_name:
            new_name = f"{class_name}::GetField_{offset:X}"
        else:
            new_name = f"GetField_{offset:X}"
        auto_named.append({**sf, 'new_name': new_name, 'pattern': 'getter', 'offset': offset})
        continue
    
    if is_boolean_check(decomp):
        auto_named.append({**sf, 'pattern': 'boolean_check', 'needs_naming': True})
        continue
    
    wrapped = is_wrapper(decomp)
    if wrapped:
        auto_named.append({**sf, 'wraps': wrapped, 'pattern': 'wrapper', 'needs_naming': True})
        continue
    
    line_count = count_lines(decomp)
    if line_count == 0:
        auto_named.append({**sf, 'pattern': 'empty/stub', 'new_name': f'Unimplemented_{sf["addr"]:06X}'})
    else:
        auto_named.append({**sf, 'pattern': 'complex', 'line_count': line_count, 'needs_naming': True})

print(f"\n\n{'=' * 70}")
print("RESULTS")
print(f"{'=' * 70}")

setter_getter_count = sum(1 for a in auto_named if a.get('pattern') in ('setter', 'getter'))
boolean_count = sum(1 for a in auto_named if a.get('pattern') == 'boolean_check')
wrapper_count = sum(1 for a in auto_named if a.get('pattern') == 'wrapper')
complex_count = sum(1 for a in auto_named if a.get('pattern') == 'complex')
empty_count = sum(1 for a in auto_named if a.get('pattern') == 'empty/stub')

print(f"  Setters/Getters auto-named: {setter_getter_count}")
print(f"  Boolean checks:              {boolean_count}")
print(f"  Wrappers:                   {wrapper_count}")
print(f"  Complex (needs manual):     {complex_count}")
print(f"  Empty/stub:                 {empty_count}")
print(f"  Decompilation failed:       {len(needs_manual)}")
print(f"  Total processed:              {len(auto_named)}")

# Save results
with open('/tmp/phase2_results_v2.json', 'w') as f:
    json.dump({
        'auto_named': auto_named,
        'needs_manual': needs_manual,
        'stats': {
            'total': len(sub_funcs),
            'setter_getter': setter_getter_count,
            'boolean_checks': boolean_count,
            'wrappers': wrapper_count,
            'complex': complex_count,
            'empty': empty_count,
            'failed': len(needs_manual),
        }
    }, f, indent=2)

print(f"\nDetailed results saved to /tmp/phase2_results_v2.json")

# Print some setters/getters as examples
if setter_getter_count > 0:
    print(f"\n=== Sample Setters/Getters ===")
    count = 0
    for a in auto_named:
        if a.get('pattern') in ('setter', 'getter') and 'new_name' in a:
            print(f"  {a['addr_hex']} {a['name']} -> {a['new_name']} ({a['pattern']} at offset 0x{a['offset']:X})")
            count += 1
            if count >= 15:
                break

# Print wrappers
if wrapper_count > 0:
    print(f"\n=== Sample Wrappers ===")
    count = 0
    for a in auto_named:
        if a.get('pattern') == 'wrapper':
            print(f"  {a['addr_hex']} {a['name']} wraps FUN_{a['wraps'][0]}")
            count += 1
            if count >= 10:
                break

# Print complex functions that might be identifiable
if complex_count > 0:
    print(f"\n=== Complex functions (need manual review) ===")
    count = 0
    for a in auto_named:
        if a.get('pattern') == 'complex' and a.get('line_count', 100) < 10:
            print(f"  {a['addr_hex']} {a['name']} ({a['line_count']} lines) [{a.get('class_name', 'free')}]")
            count += 1
            if count >= 15:
                break

print(f"\n{'=' * 70}")
print("Next steps:")
print("  1. Review /tmp/phase2_results_v2.json")
print("  2. For complex functions, decompile manually in Ghidra GUI")
print("  3. Use GhidraMCP to rename functions: rename_function_by_address")
print(f"{'=' * 70}")
