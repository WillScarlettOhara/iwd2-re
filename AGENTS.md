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
### Fields (128 total)
- CGameObject: bfield_6B → m_bLocalControl
- CVidMode: nfield_C2 → m_nRBitLoss, nfield_C6 → m_nGBitLoss, nfield_CA → m_nBBitLoss, nm_field_24 → m_dwShadowColor, nfield_DC → m_bRedrawEntireScreen
- CScreenConnection: bm_field_496 → m_bFirstRender
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
- CGameSave: nm_field_1AC → m_bArenaMode; wm_field_1B0 → m_nGroupInventoryNumber (PDB confirmed), nm_field_1B2 → m_nInventoryOffset (IWD2-specific)
- CMultiplayerSettings: bfield_AC → m_nViewedCharacter, bfield_B2 → m_bPlayerReady
- CInfinity: nm_field_20 → m_bUseDestSrc, nfield_98 → m_nSub1XOffset, nfield_9C → m_nSub1YOffset, nfield_A0 → m_nSub2XOffset, nfield_A4 → m_nSub2YOffset, nfield_A8 → m_nSub3XOffset, nfield_AC → m_nSub3YOffset (fixed bfield_AC→nfield prefix mismatch), nfield_B0 → m_nSub4XOffset, nfield_B4 → m_nSub4YOffset, nm_field_124 → m_nNewLightningFrequency, nm_field_134 → m_nCurrentWindLevel, nm_field_138 → m_nCurrentFogLevel, nm_field_13C → m_nNextWindLevel, bm_m_field_15E → m_oldRequestDualTileCode
- CChitin: nm_field_4C → m_bServicingEnabled, nfield_50 → m_bMessagesEnabled, nfield_70 → m_nIterations, nfield_B4 → m_hCopyData, nfield_BC → m_nTimerID, nfield_C0 → m_nTimerResolution, m_field_E8 → m_rClient, bfield_F8/nfield_F8 → m_bSwitchingDisplayMode (fixed type unsigned char→BOOL + offset 0xF8→0xF8→0xFC restructure), bfield_F9 → m_bEdgeScrollActive, dwPlatformId → m_opSystemPlatformId, wfield_FE → m_nCapsLockState, nm_field_110 → m_bStartUpHost, sm_field_11C → m_sStartUpPort, nm_field_124 → m_bStartUpNewGame, nm_field_128 → m_bStartUpLoadGame, bm_field_130 → m_bStartUpDirectPlayLobby, bm_field_131 → m_bStartUpGameSpyLocation, nm_field_16C → m_bMouseWheelSupport, nm_field_170 → m_nMouseWheelMessage, nm_field_1A4 → m_bIMEEnabled, nm_field_136 → m_nMaxPlayers, nm_field_19C → m_nAISleeper
- CSoundMixer: nm_field_40 → m_bStreamPlaying, nfield_C0 → m_nSoundFlags, nfield_C4 → m_bDirectSoundInit, nm_field_178 → m_bEAXSupported
- CMessage: pfield_9A → m_csPlayerRemoval; CMessageSpriteUpdate: wfield_64 → m_nArmorClass, wfield_66 → m_nACDexBonus, nfield_68 → m_nACDodgeBonus, wfield_6A → m_nACDeflectionBonus, wfield_74 → m_nPortraitIcons, pfield_76 → m_pPortraitIcons
- CDerivedStats: wm_field_8 → m_nACDexBonus, wfield_A → m_nACDodgeBonus, wfield_C → m_nACDeflectionBonus; nm_field_1E0 → m_nVisualRange (PDB), nm_field_1E4 → m_bExplore (PDB), nm_field_1E8 → m_bThrullCharm (PDB), nm_field_1EC → m_bSummonDisable (PDB); pm_field_290 → m_cImmunitiesItemEquipAlt, pm_field_2AC → m_cImmunitiesItemTypeEquipAlt; pm_field_3C8 → m_cImmunitiesProjectile, pm_field_3E4 → m_cBounceProjectileAlt, pm_field_400 → m_cImmunitiesProjectileAlt, pm_field_41C → m_cBounceProjectileLevelDec; pm_field_4AC → m_cImmunitiesSpell, pm_field_4C8 → m_cBounceSpell, pm_field_4E4 → m_cImmunitiesSchool, pfield_500 → m_cImmunitiesSecondaryType, pfield_51C → m_cBounceSchool, pfield_538 → m_cBounceSecondaryType; nm_field_124 → m_nNaturalArmorBonus (Tortoise Shell effect amount), nm_field_128 → m_nSTRBonus (effect-applied STR bonus tracker), nm_field_12C → m_nDEXBonus, nm_field_130 → m_nCONBonus, nm_field_134 → m_nINTBonus, nm_field_138 → m_nWISBonus, nm_field_13C → m_nCHRBonus
- CGameEffect: nm_field_118 → m_durationTemp (PDB confirmed, DWORD), nm_field_188 → m_sourceTarget (PDB confirmed, LONG)
- CGameEffectDeath: nm_m_field_18C → m_nDeathFlags (IWD2-specific; 1=programmatic, 0=from file)
- CGameEffectSummon: nm_m_field_18C → m_nSummonState, nm_m_field_190 → m_nSummonSide (1=ally, 2=enemy)
- CGameEffectDisguise: nm_m_field_18C → m_nDisguiseValue (init -1, opcode 437 animation value)
- IcewindCGameEffectSummon: nm_m_field_18C → m_nSummonState, nm_m_field_190 → m_nSummonSide; sub_49DB60 → SetSummonSide

### Functions (101 total)
- Icewind586B70: sub_586CA0 → IsPartyNotFull, sub_586CF0 → CanAddSprite, sub_586D60 → AddTemporary, sub_586F20 → AddPermanently, sub_586FC0 → Reinstate, sub_587190 → SyncToSprites
- CGameEffect: sub_493400 → SetSelfApply, sub_594020 → SetTimingModeFlag, sub_4C3F30 → ClampACDexBonusMinimum, sub_4B8730 → DestroyItemInSlot
- CGameEffectDamage: sub_4A7750 → RemoveStatusOnDamage
- CGameEffectStun: sub_4B2680 → ResolveStunDuration
- CGameEffectDisease: sub_4B5BF0 → ApplyDiseaseBlindingFever, sub_4B5D90 → ApplyDiseaseFeverSlow, sub_4B5E50 → ApplyDiseaseMoldTouch, sub_4B5FF0 → ScheduleDiseaseRecurrence
- IcewindCGameEffectSavingThrowChange: sub_55ED90 → IncrementSavingThrows, sub_55EE70 → SetSavingThrowsBase, sub_55EFB0 → ScaleSavingThrowsBase
- IcewindMisc: sub_584880 → DisplayEffectText, sub_585230 → AreAllies, sub_5852A0 → AreEnemies
- CGameTrigger: sub_45B950 → CanActivateTrap
- CGameAreaNotes: sub_47B150 → UpdateNote
- CGameAIBase: sub_467970 → ActionFadeColor
- CGameJournal: sub_4C7600 → WriteWrappedText
- CDerivedStats: sub_447B00 → GetSpellListByClass
- CGameArea: sub_47A060 → LoadLocalVariables, sub_47A190 → SaveMusicPosition
- CInfTileSet: sub_5D2DE0 → RenderFogOfWar
- CScreenConnection: sub_5FEB70 → OnSelectServiceProvider
- CScreenStart: sub_66F990 → StartMultiplayer, sub_6702C0 → DismissTopPopup
- CRuleTables: sub_542560 → GetMonsterEncounterRate
- CScreenKeymaps: sub_639E70 → OnDoneButtonClick, sub_63A660 → OnKeymapMouseMove, sub_63A7A0 → OnKeymapMouseClick
- CMultiplayerSettings: sub_518580 → SetViewedCharacter, sub_518660 → SetPlayerReady; bfield_AC → m_nViewedCharacter, bfield_B2 → m_bPlayerReady
- CSoundMixer: sub_7ACA10 → IsMusicPlaying, sub_7ACA30 → GetSoundChannelState
- CUIPanel: sub_4D2D20 → Activate, sub_4D2D50 → Deactivate
- CNamedCreatureVariableHashEntry: sub_550B30 → DestroyChain, sub_550CF0 → GetTail
- CScreenWorld: sub_693710 → SetKeyboardFocus, sub_695570 → SetActionPanelsActive
- CScreenWorldMap: sub_69CB40 → PulseColor
- CWorldMap: sub_55A3A0 → GetCurrentAreaIndex, sub_55A450 → GetAreaIndex
- CVisibilityMap: sub_5518A0 → InitializeEllipseArcs
- IcewindCVisualEffect: sub_586A60 → SetActive, sub_586A90 → SetGlowing, sub_586AC0 → SetTransparent
- CAIObjectType: sub_40CB20 → GetObjectByType, sub_40CCA0 → GetObject, sub_40CED0 → ResolveObjectId, sub_40D050 → GetObjectByTypeChecked, sub_40D0F0 → GetObjectChecked
- CInfGame: sub_5A9780 → VirtualKeyToChar, sub_5A97D0 → CharToVirtualKey, sub_5A9830 → GetVirtualKeyByIndex
- CGameSprite: sub_763150 → HasFeat, sub_726270 → GetFeatMode (active feat mode/rank for Power Attack/Expertise/etc.), sub_726330 → SetFeatMode, sub_726800 → GetWeaponSlot, sub_726810 → SetWeaponSet, sub_7204C0 → LoadAnimationSounds, sub_720B50 → LoadAnimationSoundEntry
- CGameSprite combat: sub_73C6A0 → GetTargetACModForDamageType, sub_73C7E0 → GetRacialLightTHAC0Mod, sub_73C8C0 → GetRacialTHAC0ModVsTarget, sub_73CA20 → GetRacialWeaponTHAC0Mod, sub_73CB10 → GetAbilityAttackMod, sub_73CC40 → GetWeaponTHAC0Bonus, sub_73D420 → GetBaseTHAC0, sub_73D440 → GetWeaponProficiencyTHAC0Mod
- CGameSprite armor: sub_724010 → GetMaxDexBonusForArmor, sub_7240A0 → GetArmorCheckPenalty, sub_724170 → GetShieldCheckPenalty, sub_724690 → IsArmorType
- CGameSprite spells: sub_724FD0 → AddClassSpell, sub_725110 → AddDomainSpell, sub_725210 → AddInnateSpell, sub_725270 → RemoveInnateSpell, sub_725330 → HasSpell, sub_7256B0 → HasDomainSpell, sub_725840 → HasInnateSpell
- CGameSprite feats/character: sub_763200 → MeetFeatRequirements, sub_763A40 → CanUpgradeFeat, sub_726570 → ApplyFeatEffects
- CGameSprite actions/UI: sub_71A0E0 → UseSpellFromButton, sub_71A550 → UseItemFromButton, sub_75F240 → SavePositionToBaseStats, sub_7615F0 → SetVisibilityRange, sub_761990 → RefreshEffects, sub_5940E0 → GetCustomButtonValue, sub_594120 → SetCustomButtonValue
- CGameSprite state/misc: sub_6FB440 → IsValidActionState, sub_724900 → IsSpellcaster, sub_724920 → IsBard, sub_724C40 → AdjustShapeshiftLevel, sub_737910 → CheckWeaponAmmunition, sub_724270 → GetShieldArcaneFailure, sub_724360 → GetArmorArcaneFailure, sub_724430 → GetArcaneSpellFailure
- Fixed nm_field_8 → m_nACDexBonus in CGameEffect.cpp (2 call sites, pre-existing prefix mismatch)
- Fixed CGameEffect.cpp prefix mismatches: bm_field_12C → m_nDEXBonus, sm_field_130 → m_nCONBonus
- Fixed IcewindCGameEffects.cpp typo: m_field_124 → m_nNaturalArmorBonus

### Mismatches Fixed (52+1 header/cpp prefix mismatches, 159 changes)
34 files corrected where .cpp used wrong type prefix for fields vs .h declarations.

## Plan
1. ~~Discover missing functions~~ (Phase 0-2 done)
2. Name/document anonymous code (sub_*, field_*) — IN PROGRESS
3. Implement incomplete code (~1,500 markers)
4. Make game playable (World Screen)
5. Modernize (cross-platform, SDL2/OpenGL)
6. Publish