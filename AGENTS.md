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

**The upstream repo compiles cleanly on VS2019 Win32 with SDK 10.0.19041. Every error in our fork was introduced by us.**

### REGLE ABSOLUE: Ne jamais casser le build

1. **ZERO tolerance.** Chaque commit sur `src/` doit compiler sans erreurs. Les warnings sont acceptables.
2. **Toute modification d'un champ de struct/classe dans un `.h` nécessite la mise à jour de TOUS les `.cpp` qui y font référence.** Utiliser `rg "oldName" src/` pour trouver toutes les occurrences.
3. **Renommage de fonction = déclaration + définition + tous les callsites dans UN SEUL commit.** Pas de commits partiels.
4. **Tout renommage dans `FileFormat.h`, `BalDataTypes.h`, `CChitin.h`, `CGameSprite.h` est EXTRÊMEMENT DANGEREUX** — ces headers sont inclus par des dizaines de fichiers.
5. **Après chaque série de renommages, builder sur Windows avant de commit.** Si pas possible, faire un commit séparé par classe pour isoler les problèmes.
6. **Les champs `field_XXX` NE DOIVENT PAS être préfixés (`nm_`, `bm_`, etc.)** sauf si on renomme TOUTES les occurrences dans TOUS les fichiers en UNE SEULE opération. Le pattern upstream est `field_XXX` sans préfixe.

### En cas d'erreur de build

- Comparer avec upstream: `git diff upstream/main -- src/`
- Si la cause n'est pas évidente: `git checkout upstream/main -- src/LeFichier.h` pour revenir à l'upstream
- Ne jamais "fixer" en cascade sans comprendre la cause racine

## Naming Conventions
- Address comments: `// 0x7D14F0` above every function/variable
- Class offsets: `/* 0044 */ RESREF groundIcon;`
- Unnamed functions: `sub_NNNNNN` (address-based placeholder)
- Unnamed fields: `field_X` (offset-based placeholder, NO type prefix — match upstream)
- Named fields: `m_` prefix for class members, Hungarian notation (`n`, `b`, `p`)
- Upstream convention: `field_XXX` (NO type prefix like `nm_`, `bm_`)

## Rename Strategy

**CRITICAL: Ghidra DB is the source of truth.** Always rename in Ghidra FIRST, save, then sync to C++. If the C++ source ever needs to be reset (build break, upstream rebase), the Ghidra DB preserves all renames.

### Rename workflow (DO NOT SKIP STEPS)

```
# 1. Rename in Ghidra DB (SOURCE OF TRUTH)
ghidrasql ... -q "UPDATE funcs SET name = 'NewName' WHERE address = 0xNNNNNN;"

# 2. SAVE immediately
ghidrasql ... -q "SELECT save_database();"

# 3. THEN sync to C++ source
rg "sub_NNNNNN" src/              # find all occurrences
# Update .h declaration + .cpp definition + all callsites
```

### Research
1. **GhidraSQL** — Decompile function, trace dataflow via xrefs
2. **BG2EE PDB** — Check `/tmp/bg2_pdb_members.txt` for original name
3. **GemRB** — Check for semantic equivalent
4. **NearInfinity / IESDP** — IWD2-specific structure/opcode definitions
5. **Source context** — Usage patterns in .cpp files
6. **CRITICAL: After name is chosen, find ALL references** with `rg "oldName" src/` before renaming
7. **CRITICAL: Rename ALL occurrences atomically** — declaration + definition + all callsites in one commit

Priority: small classes first → CGameSprite last (most complex, most fields)

### Safe rename checklist
```
[ ] Researched proper name via IESDP/PDB/GemRB
[ ] Renamed in Ghidra DB: UPDATE funcs SET name = ...
[ ] Saved Ghidra DB: SELECT save_database();
[ ] Found ALL src/ occurrences with: rg "oldName" src/
[ ] Updated .h declaration
[ ] Updated .cpp definition + all callsites
[ ] Built on Windows: cmake --build build --config Debug
[ ] Zero NEW errors (only pre-existing warnings)
[ ] Commit with message: "rename: ClassName::methodName"
```

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
