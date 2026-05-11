# AGENTS.md — iwd2-re Project

Reverse engineering of Icewind Dale 2 — recover the lost source code.

## Repo
- **Upstream**: https://github.com/alexbatalov/iwd2-re
- **Fork**: https://github.com/WillScarlettOhara/iwd2-re

## Current Status
- Functions: 80% (~8,200 / ~10,000)
- Code: 53% (~2M / ~3.8M lines)
- Game: NOT playable (only UI screens work)
- sub_ remaining: ~200 unique functions
- field_ remaining: ~640 unnamed fields
- TODO: ~2199 | FIXME: ~427

## Workflow — Iterative RE Loop

For each unnamed function/field, execute:

1. **Decompile**: `ghidrasql -q "SELECT text FROM pseudocode WHERE func_addr = 0xNNNNNN;"`
2. **Context**: Read relevant Memory Nodes (see below) + PDB/GemRB/IW2 source
3. **Annotate**: Rename variables, apply types, add inline comments
4. **Update**: Apply changes to Ghidra DB via SQL, then sync to C++ source

## Memory Nodes

Markdown files for persistent knowledge. Create one per sub-system. Always read before analyzing a function in that sub-system, and write back when you discover new structs/globals/patterns.

**Existing**: (none yet — create as you go)
- Pattern: `/home/wills/projects/IWD2-RE/iwd2-re/decomp_ref/<subsystem>.md`

## Reference Sources

### GhidraSQL (PRIMARY — decompilation & DB queries)
```bash
ghidrasql --ghidra "$GHIDRA_INSTALL_DIR" \
  --project ~/ghidra_projects/IWD2 --project-name IWD2 \
  --program IWD2.exe --no-analyze -q "SQL"
```

| Query | SQL |
|-------|-----|
| Decompile | `SELECT text FROM pseudocode WHERE func_addr = 0x5D2DE0;` |
| List FUN_* | `SELECT name, printf('0x%X', address) AS addr FROM funcs WHERE name LIKE 'FUN_00%' ORDER BY address;` |
| Xrefs to addr | `SELECT * FROM xrefs WHERE to_ea = 0x5D2DE0;` |
| Callers | `SELECT name FROM callers WHERE dst = 'RenderFogOfWar';` |
| Strings | `SELECT * FROM strings WHERE s LIKE '%error%';` |
| Rename func | `UPDATE funcs SET name = 'NewName' WHERE address = 0x5D2DE0;` |
| Retype var | `UPDATE decomp_lvars SET type = 'int' WHERE func_addr = 0x5D2DE0 AND name = 'local_8';` |
| Save | `SELECT save_database();` |

**Rules**: `pseudocode`, `decomp_lvars`, `decomp_tokens` MUST filter by `func_addr`. Never unbounded.

### BG2EE PDB Symbols
- **File**: `/home/wills/projects/bg2-symbols/Baldur.pdb` (BG2EE v2.5.16.6)
- **Extracted**: `/tmp/bg2_pdb_members.txt`, `/tmp/bg2_pdb_types.txt`
- BG2 shares ~90% engine classes — names are authoritative, offsets differ

### GemRB (secondary cross-reference)
- **Path**: `/home/wills/projects/gemrb/`
- CGameSprite→Actor, CInfGame→Game, CGameArea→Map, CGameEffect→Effect, CInfinity→Interface

### Other References
- **NearInfinity**: `/home/wills/projects/NearInfinity/` — IWD2 file format structures (CreResource.java for CRE v2.2, AreResource.java, Effect2.java)
- **IESDP**: `/tmp/iesdp/iesdp-gh-pages/` — IWD2 effects (353 opcodes), STATS.IDS (106 entries), IDS files, file formats

## Tooling
- Ghidra 12.0.4 + LibGhidraHost extension
- GhidraSQL CLI at `/home/wills/projects/IWD2-RE/ghidrasql/build/bin/` (in PATH via .zshrc)
- Binary: IWD2.exe (28,024 functions in Ghidra project)
- `GHIDRA_INSTALL_DIR=/opt/ghidra_12.0.4_PUBLIC`
- `JAVA_HOME=/usr/lib/jvm/java-21-openjdk` (required for Ghidra)
- **protoc 29.6** at `/home/wills/tools/protoc-29.6/bin/protoc` (needed if rebuilding libghidra)

## Directory Structure
- `src/` — Flat layout, ~202 .cpp + ~209 .h, MFC Hungarian naming
- `src/music/` — Miles Sound System
- `third_party/` — zlib, DirectPlay headers
- `scripts/` — Analysis/rename scripts
- Build: Visual Studio 2019 Win32, C++17, MFC, DirectDraw

## Scripts (`scripts/`)
| Script | Purpose |
|--------|---------|
| `fix_mismatches.py` | Fix header/cpp field prefix mismatches (`--apply`) |
| `phase5_catalog.py` | Build field catalog |
| `phase5_batch_rename.py` | Class-scoped safe batch rename |
| `phase5_auto_name_fields.py` | Auto-inference from getter/setter patterns |
| `cross_ref.py` | Cross-reference analysis |

## Build Safety — CRITICAL RULES

Every rename/edit MUST pass these checks. Breaking any of them causes cascading MSVC build failures that take hours to fix.

### 1. No Duplicate Field Names
Before renaming a field, verify the new name does NOT already exist in the same class:
```bash
grep "m_cMyNewName" src/<Class>.h
```
**Root cause of CDerivedStats.h crash**: `pm_field_3C8` renamed to `m_cImmunitiesProjectile` which already existed at offset 0x0188. MSVC treats identical field names as redefinition → 50+ compilation units fail.

### 2. No Value Members Without #include
If a class contains `CSound m_sndFoo`, `CVidCell m_vidBar`, or `CVidBitmap m_vbBaz` as **value members** (not pointers), the header must include the full class definition:
```cpp
#include "CSound.h"
#include "CVidCell.h"
#include "CVidBitmap.h"
```
Forward declarations (`class CVidCell;`) suffice for pointers/references, but value members need the complete type. Without the include, MSVC fails with `C2079: uses undefined class`.

### 3. Header/cpp Field Name Consistency
When renaming fields in `.h`, also rename them in **every** `.cpp` that uses them. The `fix_mismatches.py` script catches prefix mismatches (e.g., `.cpp` uses `nfield_54A8` while `.h` declares `m_field_54A8`), but it does NOT catch:
- Completely renamed fields (e.g., `field_10` in .h → `m_nSomething` but .cpp still uses `field_10`)
- Fields accessed via `->` or `::` notation
**Always check**: after any field rename, search all `.cpp` for the old name and verify zero matches remain.

### 4. Post-Rename Verification
```bash
# Check for any remaining references to old field names in .cpp
grep -rn "old_field_name" src/ --include="*.cpp"

# Check for duplicate field declarations in headers
grep -n "m_yourNewName" src/Class.h  # must appear exactly once
```

## Naming Conventions
- Address comments: `// 0x7D14F0` above every function/variable
- Class offsets: `/* 0044 */ RESREF groundIcon;`
- Unnamed functions: `sub_NNNNNN` (address-based placeholder)
- Unnamed fields: `field_X` (offset-based placeholder)
- MFC Hungarian: `m_` members, `n` ints, `b` bools, `p` pointers
- Field prefixes: `nm_`=int, `bm_`=bool/byte, `sm_`=string, `pm_`=pointer, `wm_`=short

## Rename Strategy
1. **GhidraSQL** — Decompile function, trace dataflow via xrefs
2. **BG2EE PDB** — Check `/tmp/bg2_pdb_members.txt` for original name
3. **GemRB** — Check for semantic equivalent
4. **NearInfinity / IESDP** — IWD2-specific structure/opcode definitions
5. **Source context** — Usage patterns in .cpp files

Priority: small classes first → CGameSprite last (most complex, most fields)

## Key Commands
```bash
# GhidraSQL decompilation
ghidrasql --ghidra "$GHIDRA_INSTALL_DIR" --project ~/ghidra_projects/IWD2 \
  --project-name IWD2 --program IWD2.exe --no-analyze \
  -q "SELECT text FROM pseudocode WHERE func_addr = 0x5D2DE0;"

# Count remaining sub_ functions
ghidrasql ... -q "SELECT COUNT(*) FROM funcs WHERE name LIKE 'FUN_00%';"

# Extract PDB symbols
llvm-pdbutil dump -types /home/wills/projects/bg2-symbols/Baldur.pdb > /tmp/bg2_pdb_types.txt

# Fix header/cpp mismatches
python3 scripts/fix_mismatches.py         # dry run
python3 scripts/fix_mismatches.py --apply  # apply
```

## Plan
1. ~~Discover missing functions~~ (Phase 0-2 done)
2. Name/document anonymous code (sub_*, field_*) — IN PROGRESS
3. Implement incomplete code (~1,500 markers)
4. Make game playable (World Screen)
5. Modernize (cross-platform, SDL2/OpenGL)
6. Publish
