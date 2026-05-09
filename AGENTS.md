# AGENTS.md — iwd2-re Project

Reverse engineering of Icewind Dale 2 — recover the lost source code.

## Repo
- **Upstream**: https://github.com/alexbatalov/iwd2-re
- **Fork**: https://github.com/WillScarlettOhara/iwd2-re

## Current Status
- Functions: 80% (~8,200 / ~10,000)
- Code: 53% (~2M / ~3.8M lines)
- Game: NOT playable (only UI screens work)
- sub_ remaining: ~781 references (~243 unique functions)
- field_ remaining: ~648 unique unnamed fields
- TODO: ~2199 | FIXME: ~427

## Reference Sources
These external sources provide original developer names for reverse engineering:

### BG2EE PDB Symbols (PRIMARY)
- **File**: `/home/wills/projects/bg2-symbols/Baldur.pdb` (29MB, BG2EE v2.5.16.6)
- **Extracted**: `/tmp/bg2_pdb_members.txt` (834 lines, 13,474 unique member names)
- **Full dump**: `/tmp/bg2_pdb_types.txt` (550K lines, all PDB types)
- **Extraction**: `llvm-pdbutil dump -types Baldur.pdb > /tmp/bg2_pdb_types.txt`
- **Coverage**: BG2 shares ~90% of engine classes with IWD2. Offsets differ but NAMES are original.
- **Key classes with member data**: CGameSprite (100+ members), CSpawn, CGameEffect, CGameAreaNotes, CGameTrigger, CGameContainer, CDerivedStatsTemplate, CPersistantEffect, CInfinity fields available via field_list indices.

### GemRB (SECONDARY)
- **Path**: `/home/wills/projects/gemrb/`
- **Usage**: Class/member name cross-reference. Names often differ from originals but semantics match.
- **Key mappings**: CGameSprite→Actor, CInfGame→Game, CGameArea→Map, CGameEffect→Effect, CInfinity→Interface, CSpawn→Spawn/CritterEntry (IniSpawn), CGameTrigger→InfoPoint, CGameContainer→Container, CDerivedStats→PCStats

### Ghidra MCP
- **Method**: Connect via `ghidra-mcp_connect_instance` with project "IWD2"
- **Requires**: Ghidra running with JAVA_HOME=/usr/lib/jvm/java-21-openjdk
- **Usage**: `analyze_function_complete` for decompilation, `analyze_dataflow` for field inference
- **Socket**: `/run/user/1000/ghidra-mcp/ghidra-*.sock`

## Tooling
- Ghidra 12.0.4 + GhidraMCP headless (Docker, port 8089)
- Binary: /home/wills/Games/Heroic/Icewind Dale 2/IWD2.exe (28,362 functions identified)
- Auth token: `iwd2ee-re-tools-2026`
- Source reference: /home/wills/projects/IWD2-RE/iwd2-re/

## Directory Structure
- `src/` — Flat layout, ~202 .cpp + ~209 .h, organized by naming convention
- `src/music/` — Audio subsystem (Miles Sound System)
- `third_party/` — zlib, DirectPlay headers
- `scripts/` — Rename/analysis scripts (see below)
- Build: Visual Studio 2019 Win32, C++17, MFC, DirectDraw

## Scripts (`scripts/`)
- `fix_mismatches.py` — Fix header/cpp field prefix mismatches (run with `--apply`)
- `phase5_catalog.py` — Build field catalog (50 easy, 38 medium, 9 hard classes)
- `phase5_batch_rename.py` — Class-scoped safe batch rename with dry-run mode
- `phase5_cgsprite.py` — CGameSprite-specific field analysis
- `phase5_auto_name_fields.py` — Auto-inference from getter/setter patterns
- `phase2_*.py` — Phase 2 rename application scripts
- `phase3_add_docs.py` — Header documentation comments
- `cross_ref.py` — Cross-reference analysis
- `dump_decomp.py` — Ghidra decompilation dump

## Key Commands
```bash
# Ghidra MCP Docker
cd /home/wills/projects/IWD2-RE/ghidra-mcp/docker && docker compose up -d

# Launch Ghidra (needs JDK 21)
JAVA_HOME=/usr/lib/jvm/java-21-openjdk /opt/ghidra/ghidraRun &

# Extract PDB symbols
llvm-pdbutil dump -types /home/wills/projects/bg2-symbols/Baldur.pdb > /tmp/bg2_pdb_types.txt

# Fix header/cpp mismatches (dry run, then --apply)
python3 scripts/fix_mismatches.py         # dry run
python3 scripts/fix_mismatches.py --apply  # apply changes
```

## Naming Conventions
- Address comments: `// 0x7D14F0` above every function/variable
- Class offsets: `/* 0044 */ RESREF groundIcon;`
- Unnamed functions: `sub_NNNNNN` (address-based placeholder)
- Unnamed fields: `field_X` (offset-based placeholder)
- MFC Hungarian: `m_` members, `n` ints, `p` pointers
- Field prefixes: `nm_field_X` = int, `bm_field_X` = byte/bool, `sm_field_X` = string, `pm_field_X` = pointer, `wm_field_X` = short, `m_field_X` = general
- Double-prefix pattern: `nm_m_field_X` (first letter = Hungarian type, `m_` = member)

## Rename Strategy (Phase 5)
For each unnamed field:
1. **BG2EE PDB** — Check `/tmp/bg2_pdb_members.txt` for original name (offsets may differ, names are authoritative)
2. **GemRB** — Check for semantic equivalent
3. **Ghidra MCP** — Decompile getters/setters to confirm offset→name mapping
4. **Source context** — Infer from usage patterns in .cpp files

Priority order: small classes first (few fields, high confidence) → CGameSprite last (101 fields, complex)

## Completed Renames
### Fields (98 total)
- CProjectile::nm_field_182 → m_secondaryProjectileId
- CResTile::sfield_58 → m_cResRef (also fixed cpp bug nfield_58)
- CSound::nm_field_3C → m_bFrequencyInitialized
- CVidCell::nfield_C8 → m_bCyclic
- CResCache::nm_field_110 → m_bCacheLocked, nm_field_118 → m_bCopyError, sm_field_10C → m_sTempFileName, nm_field_114 → m_bCopyInProgress
- Icewind586B70::Entry::m_field_0 → m_targetId, m_field_4 → m_controllerId, m_field_8 → m_bPermanent
- CAlias::bm_field_4 → m_bParsed, sm_field_8 → m_sAliasPrefix, pfield_C → m_lReplacementPaths
- CGameSpy::bm_field_0 → m_bWinsockInitialized, bm_field_1 → m_bConnected
- CUIControlEdit: nm_field_350 → m_nTextOffsetX, nm_field_354 → m_nTextOffsetY, wm_field_85C → m_nEditMode, sm_field_868 → m_sTextOriginal, nm_field_89C → m_bConfirmOnExit, bm_field_872 → m_nCommandHistoryIndex, bm_field_873 → m_nCommandHistorySize, bm_field_8A0 → m_bDefocusOnReturn, bm_field_8A1 → m_bFontNeedsLoading
- CVisibilityMap: bm_field_4 → m_nEllipseArcCountPrimary, wfield_E → m_nEllipseArcWidth, wm_field_10 → m_nEllipseArcHeight, pm_field_12 → m_pEllipseArcPixelsPrimary, pm_field_16 → m_pEllipseArcPixelsSecondary, bm_field_1A → m_nEllipseArcCountSecondary
- CDerivedStatsTemplate: wfield_6 → m_nArmorClass
- CGameAreaNotes: bm_field_8D → m_nCurrentIcon, bm_field_8E → m_bAddingNote
- CInfinity: nm_field_20 → m_bUseDestSrc, nfield_98 → m_nSub1XOffset, nfield_9C → m_nSub1YOffset, nfield_A0 → m_nSub2XOffset, nfield_A4 → m_nSub2YOffset, nfield_A8 → m_nSub3XOffset, nfield_AC → m_nSub3YOffset (fixed bfield_AC→nfield prefix mismatch), nfield_B0 → m_nSub4XOffset, nfield_B4 → m_nSub4YOffset, nm_field_124 → m_nNewLightningFrequency, nm_field_134 → m_nCurrentWindLevel, nm_field_138 → m_nCurrentFogLevel, nm_field_13C → m_nNextWindLevel, bm_m_field_15E → m_oldRequestDualTileCode
- CChitin: nm_field_4C → m_bServicingEnabled, nfield_50 → m_bMessagesEnabled, nfield_70 → m_nIterations, nfield_B4 → m_hCopyData, nfield_BC → m_nTimerID, nfield_C0 → m_nTimerResolution, m_field_E8 → m_rClient, bfield_F8/nfield_F8 → m_bSwitchingDisplayMode (fixed type unsigned char→BOOL + offset 0xF8→0xF8→0xFC restructure), bfield_F9 → m_bEdgeScrollActive, dwPlatformId → m_opSystemPlatformId, wfield_FE → m_nCapsLockState, nm_field_110 → m_bStartUpHost, sm_field_11C → m_sStartUpPort, nm_field_124 → m_bStartUpNewGame, nm_field_128 → m_bStartUpLoadGame, bm_field_130 → m_bStartUpDirectPlayLobby, bm_field_131 → m_bStartUpGameSpyLocation, nm_field_16C → m_bMouseWheelSupport, nm_field_170 → m_nMouseWheelMessage, nm_field_1A4 → m_bIMEEnabled, nm_field_136 → m_nMaxPlayers, nm_field_19C → m_nAISleeper
- CSoundMixer: nm_field_40 → m_bStreamPlaying, nfield_C0 → m_nSoundFlags, nfield_C4 → m_bDirectSoundInit, nm_field_178 → m_bEAXSupported
- CMessage: pfield_9A → m_csPlayerRemoval; CMessageSpriteUpdate: wfield_64 → m_nArmorClass, wfield_66 → m_nACDexBonus, nfield_68 → m_nACDodgeBonus, wfield_6A → m_nACDeflectionBonus, wfield_74 → m_nPortraitIcons, pfield_76 → m_pPortraitIcons
- CDerivedStats: wm_field_8 → m_nACDexBonus, wfield_A → m_nACDodgeBonus, wfield_C → m_nACDeflectionBonus; pm_field_290 → m_cImmunitiesItemEquipAlt, pm_field_2AC → m_cImmunitiesItemTypeEquipAlt; pm_field_3C8 → m_cImmunitiesProjectile, pm_field_3E4 → m_cBounceProjectileAlt, pm_field_400 → m_cImmunitiesProjectileAlt, pm_field_41C → m_cBounceProjectileLevelDec; pm_field_4AC → m_cImmunitiesSpell, pm_field_4C8 → m_cBounceSpell, pm_field_4E4 → m_cImmunitiesSchool, pfield_500 → m_cImmunitiesSecondaryType, pfield_51C → m_cBounceSchool, pfield_538 → m_cBounceSecondaryType

### Functions (6 total)
- Icewind586B70: sub_586CA0 → IsPartyNotFull, sub_586CF0 → CanAddSprite, sub_586D60 → AddTemporary, sub_586F20 → AddPermanently, sub_586FC0 → Reinstate, sub_587190 → SyncToSprites

### Mismatches Fixed (52+1 header/cpp prefix mismatches, 159 changes)
34 files corrected where .cpp used wrong type prefix for fields vs .h declarations.

## Plan
1. ~~Discover missing functions~~ (Phase 0-2 done)
2. Name/document anonymous code (sub_*, field_*) — IN PROGRESS
3. Implement incomplete code (~1,500 markers)
4. Make game playable (World Screen)
5. Modernize (cross-platform, SDL2/OpenGL)
6. Publish