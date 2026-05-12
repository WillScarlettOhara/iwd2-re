# External References

All resources recoverable from this repo. Run `data/restore.ps1` after clone.

## Bundled in repo

### PDB Extracted Texts
- `data/pdb/bg2_pdb_extracted.zip` (7.5 MB) — contains:
  - `bg2_pdb_types.txt` (72 MB) — LF_CLASS, LF_FIELDLIST, LF_MEMBER
  - `bg2_pdb_symbols.txt` (45 MB) — function/variable symbols
  - `bg2_pdb_globals.txt` (9 MB) — global variables
- Source: BG2EE v2.5.16.6 Baldur.pdb
- Extraction command: `llvm-pdbutil dump -types Baldur.pdb > bg2_pdb_types.txt`
- Mapping: ~90% engine class field names are identical, offsets differ

### Ghidra Projects
- `data/ghidra/IWD2_rep.zip` (60 MB) — IWD2.rep repository (main RE target, 28,024 functions)
- `data/ghidra/BG2EE_rep.zip` (70 MB) — BG2EE.rep repository (cross-reference)
- Restore to: `C:\ghidra_projects\IWD2\IWD2.rep\` and `C:\ghidra_projects\BG2EE\BG2EE.rep\`

### Git Submodules
- `refs/gemrb/` — https://github.com/gemrb/gemrb (CGameSprite→Actor, CInfGame→Game, CGameArea→Map)
- `refs/NearInfinity/` — https://github.com/NearInfinityBrowser/NearInfinity (IWD2 file formats: CreResource.java, AreResource.java, Effect2.java)
- `refs/iesdp/` — https://github.com/Gibberlings3/iesdp (IWD2 effects 353 opcodes, STATS.IDS 106 entries)

Init submodules: `git submodule update --init --recursive`

## Not bundled (too large / require installation)

- **Baldur.pdb** (29 MB) — `C:\projects\bg2-symbols\Baldur.pdb`
- **Ghidra 12.0.4** — `C:\ghidra_dist\ghidra_12.0.4_PUBLIC`
- **GhidraSQL** — `C:\ghidrasql-repo\build\bin\Release\ghidrasql.exe`
- **Visual Studio 2019** — Win32, C++17, MFC, DirectDraw, SDK 10.0.19041

## C++ ↔ BG2 PDB class correspondences

| IWD2 Class | BG2EE Class | Key offset diff |
|---|---|---|
| CInfGame | CInfGame @ 0x11C8E | BG2 sizeof=24376, IWD2 sizeof=19124 |
| CBaldurChitin | CBaldurChitin | — |
| CGameSprite | CGameSprite | most complex, last to rename |
