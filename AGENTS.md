# iwd2-re Agent Rules

Repo: `github.com/WillScarlettOhara/iwd2-re` (fork of alexbatalov)

## Build Safety — ZERO TOLERANCE
- Every `src/` commit must compile clean on VS2019 Win32.
- Rename field → update `.h` + ALL `.cpp` refs in ONE commit. `rg "oldName" src/`
- Rename function → Ghidra DB FIRST, save, then sync `.h` + `.cpp` + callsites atomically.
- Danger zone: `FileFormat.h`, `BalDataTypes.h`, `CChitin.h`, `CGameSprite.h`.
- Never prefix `field_XXX` with type letters (`nm_`, etc.).
- Break? `git diff upstream/main -- src/` or `git checkout upstream/main -- file`.

## Naming
- Functions: `sub_NNNNNN` placeholder → `ClassName::methodName`.
- Fields: `field_XX` placeholder → `m_typeName` (Hungarian, no type prefix on placeholder).
- Addresses: `// 0x7D14F0` above each function.

## Rename Workflow (functions)
1. `UPDATE funcs SET name = 'NewName' WHERE address = 0xNNNNNN;`
2. `SELECT save_database();`
3. Sync to C++ source in one commit.

## Rename Workflow (fields)
1. Match class in `bg2_pdb_types.txt` (BG2EE PDB offsets differ, names match).
2. Rename `field_XX` in `.h` + all `.cpp` in ONE commit.
3. Build before push.

## GhidraSQL — Correct Startup
- **First import / empty DB**: `C:\ghidra_projects\IWD2\analyze_and_host.bat`
  - Uses `--binary ... --analyze --keep-host`
- **Later starts**: `C:\ghidra_projects\IWD2\start_ghidrasql.bat`
  - Uses `--program IWD2.exe --initial-program IWD2.exe --no-analyze`
- **Sanity gate**: `SELECT COUNT(*) FROM funcs;` must be `> 0`
  - If `0`: wrong startup or empty project. Re-import + analyze.
- Never kill process. Always `POST /shutdown`.

## Key Commands
```powershell
# Decompile
ghidrasql --url http://127.0.0.1:8081 -q "SELECT text FROM pseudocode WHERE func_addr = 0x5D2DE0;"
# Save
ghidrasql --url http://127.0.0.1:8081 -q "SELECT save_database();"
# First import + analyze
C:\ghidra_projects\IWD2\analyze_and_host.bat
# Start existing analyzed program
C:\ghidra_projects\IWD2\start_ghidrasql.bat
# Shutdown (never kill process)
curl -X POST http://127.0.0.1:8081/shutdown
# Validate DB
curl -X POST http://127.0.0.1:8081/query --data "SELECT COUNT(*) FROM funcs;"
# Fix header/cpp mismatches
python scripts/fix_mismatches.py --apply
```

## Reference Paths
- PDB types: `C:/projects/bg2-symbols/bg2_pdb_types.txt`
- GemRB: `C:/projects/gemrb/`
- NearInfinity: `C:/projects/NearInfinity/`
- IESDP: `C:/projects/iesdp/`

## Ghidra Tools
- **Before starting server**: delete stale lock files first: `rm -f "C:/ghidra_projects/IWD2/IWD2.lock" "C:/ghidra_projects/IWD2/IWD2.lock~"`
- `bash scripts/ghidra_info.sh 0xADDR` — show func + bookmarks + comments for address
- `python scripts/ghidra_re.py decomp 0xADDR` — decompile function
- `python scripts/ghidra_re.py func --address 0xADDR --name ClassName::method` — rename
- `python scripts/ghidra_re.py bookmark --type Note 0xADDR Category "text"` — add bookmark
- `python scripts/ghidra_re.py comment --source plate 0xADDR "text"` — add comment
- `curl -X POST http://127.0.0.1:8081/query --data "SELECT save_database();"` — persist changes
- `curl -X POST http://127.0.0.1:8081/shutdown` — graceful stop

## Naming Rules
- Rename only: source code match OR decomp unambiguous clarity
- Speculative → `FUN_` + bookmark with `Analysis` category
- Search: `SELECT name, printf('0x%X', address) FROM funcs WHERE name LIKE 'FUN_%' AND address >= 0x... AND size >= 50;`

## Current Milestone
Phase 2: name remaining `sub_` (~200) and `field_` (~640). Priority: small classes first, `CGameSprite` last.
