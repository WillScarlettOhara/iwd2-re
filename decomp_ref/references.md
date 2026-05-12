# External References

Recoverable resources used in this RE project.

## Primary — BG2EE PDB

- **PDB file**: `C:\projects\bg2-symbols\Baldur.pdb` (BG2EE v2.5.16.6)
- **Extracted**:
  - `C:\projects\bg2-symbols\bg2_pdb_types.txt` (72 MB) — LF_CLASS, LF_FIELDLIST, LF_MEMBER, etc.
  - `C:\projects\bg2-symbols\bg2_pdb_symbols.txt` (45 MB) — function/variable symbols
  - `C:\projects\bg2-symbols\bg2_pdb_globals.txt` (9 MB) — global variables
- **Extraction**: `llvm-pdbutil dump -types Baldur.pdb > bg2_pdb_types.txt`
- **Mapping**: ~90% engine class field names are identical, offsets differ

## Secondary — Ghidra Projects

- **IWD2**: `C:\ghidra_projects\IWD2` — main RE target, 28,024 functions
- **BG2EE**: `C:\ghidra_projects\BG2EE` — cross-reference for field/function names
- **Server**: `http://127.0.0.1:8081` (ghidrasql), graceful shutdown with `/shutdown`

## Tertiary — Other Sources

- **GemRB**: `C:\projects\gemrb\` — CGameSprite→Actor, CInfGame→Game, CGameArea→Map
- **NearInfinity**: `C:\projects\NearInfinity\` — IWD2 file formats (CreResource.java, AreResource.java, Effect2.java)
- **IESDP**: `C:\projects\iesdp\` — IWD2 effects (353 opcodes), STATS.IDS (106 entries)

## C++ ↔ BG2 PDB class correspondences

| IWD2 Class | BG2EE Class | Key offset diff |
|---|---|---|
| CInfGame | CInfGame @ 0x11C8E | BG2 sizeof=24376, IWD2 sizeof=19124 |
| CBaldurChitin | CBaldurChitin | — |
| CGameSprite | CGameSprite | most complex, last to rename |
