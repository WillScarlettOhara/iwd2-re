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

## Key Commands
```powershell
# Decompile
ghidrasql --url http://127.0.0.1:8081 -q "SELECT text FROM pseudocode WHERE func_addr = 0x5D2DE0;"
# Save
ghidrasql --url http://127.0.0.1:8081 -q "SELECT save_database();"
# Start server
C:\ghidra_projects\IWD2\start_ghidrasql.bat
# Shutdown (never kill process)
curl -X POST http://127.0.0.1:8081/shutdown
# Fix header/cpp mismatches
python scripts/fix_mismatches.py --apply
```

## Reference Paths
- PDB types: `C:/projects/bg2-symbols/bg2_pdb_types.txt`
- GemRB: `C:/projects/gemrb/`
- NearInfinity: `C:/projects/NearInfinity/`
- IESDP: `C:/projects/iesdp/`

## Current Milestone
Phase 2: name remaining `sub_` (~200) and `field_` (~640). Priority: small classes first, `CGameSprite` last.
