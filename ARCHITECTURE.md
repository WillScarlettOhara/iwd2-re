# Architecture

This document describes the high-level architecture of the Icewind Dale II engine.

## Overview

The Infinity Engine is a real-time pauseable RPG engine with the following characteristics:
- **2D isometric** rendering with tile-based maps
- **Party-based** (up to 6 characters)
- **AI-driven** through scripts (`.BCS` files)
- **Rule-based** using D&D 3rd Edition mechanics
- **Message-passing** between game objects

## Directory Structure

```
src/                    # All source files (flat layout)
  music/               # Audio subsystem (Miles Sound System)
  CAI*.cpp/h           # AI/Scripting system
  CGame*.cpp/h         # Game objects and state
  CScreen*.cpp/h       # UI screens
  CRes*.cpp/h          # Resource loading
  CInf*.cpp/h          # Core engine extensions
  CVid*.cpp/h          # Video/rendering

scaffold/              # Auto-generated function declarations
  *_scaffold.h         # Cross-referenced from Ghidra binary
  00_uncategorized.h   # Unmatched functions

third_party/           # External dependencies
  directplay/          # DirectPlay headers (multiplayer)
  zlib/                # Compression
```

## Core Systems

### 1. Application Framework (CChitin)

**CChitin** is the base application class:
- Message pump and game loop
- Window management
- Timer updates (25 fps)
- Subsystem initialization order

**CBaldurChitin** extends it for Baldur's Gate / Icewind Dale:
- Game-specific constants
- Message handler routing
- Multiplayer integration

### 2. Rendering (CInfinity + CVid*)

**CInfinity** — The world renderer:
- Tile map rendering (`.WED` + `.TIS` files)
- Sprite animation (`.BAM` files)
- Lighting and shadows
- Scrollable viewport
- Search map (pathfinding grid)

**CVidCell** — Sprite animation player
- Frame-based animation
- Color cycling
- Alpha blending

**CVidBitmap** — Static image display

### 3. Game State (CInfGame)

Singleton holding all persistent state:
- **Party**: 6 playable characters
- **Areas**: loaded/unloaded as player moves
- **Journal**: quest entries and notes
- **World Map**: discovered locations
- **Multiplayer**: player settings
- **Saves**: serialization/deserialization

### 4. Game Objects (CGameAIBase hierarchy)

All interactive objects inherit from **CGameAIBase**:

```
CGameAIBase (action queue, scripts, triggers)
  ├── CGameSprite (creatures/characters)
  ├── CGameDoor (doors)
  ├── CGameTrigger (script triggers)
  ├── CGameArea (area controller)
  ├── CGameContainer (chests/containers)
  ├── CGameStatic (static objects)
  └── CGameTiledObject (tiled objects)
```

Each object has:
- **Position** (x, y on tile map)
- **AI Type** (class, race, alignment for matching)
- **Script** (`.BCS` file reference)
- **Action Queue** (pending actions)
- **Triggers** (pending events)

### 5. AI/Scripting System

**CAIScript** — Script interpreter:
- Parses `.BCS` files (Condition = Action pairs)
- Evaluates conditions every AI tick
- Queues actions on matching objects

**CAIAction** — Individual actions:
- `MoveToObject`, `Attack`, `Dialogue`, `CastSpell`, etc.
- Each action has parameters (target, duration, etc.)
- Return codes: `ACTION_DONE`, `ACTION_ONGOING`, `ACTION_ERROR`

**CAIObjectType** — Object matcher:
- Filters objects by class, race, alignment, etc.
- Used for script targeting (`NearestEnemyOf()`)

### 6. UI Screens (CScreen*)

Each screen is a self-contained subsystem:
- **CScreenWorld** — Main gameplay (NOT working yet)
- **CScreenCharacter** — Character sheet
- **CScreenInventory** — Item management
- **CScreenSpellbook** — Spell memorization
- **CScreenJournal** — Quest log
- **CScreenMap** — Auto-map

Screens use:
- **CUIManager** — UI element container
- **CUIPanel** — Layout panels
- **CUIControl*** — Buttons, labels, text fields, scrollbars

### 7. Resource System (CRes*, CDimm)

**CRes** — Base resource class:
- `.ARE` — Areas
- `.CRE` — Creatures
- `.ITM` — Items
- `.BAM` — Animations
- `.WED` — World map data
- `.DLG` — Dialogues
- `.STO` — Stores

**CDimm** — Resource manager:
- Loads files from game directory
- Caches frequently used resources
- Unloads unused resources

### 8. Effects System (CGameEffect, CDerivedStats)

**CGameEffect** — Individual magical/physical effects:
- Duration (instant, temporary, permanent)
- Modifiers (strength, AC, saves, etc.)
- Flags and dice rolls

**CDerivedStats** — Calculated statistics:
- Base stats + equipment + effects
- THAC0, AC, saving throws, resistances
- Updated every tick when effects change

### 9. Message System (CMessage)

**CMessage** — Inter-object communication:
- `CMessageMoveToPoint` — Movement request
- `CMessageAttack` — Combat initiation
- `CMessageDamage` — Apply damage
- `CMessageSetTrigger` — Event notification

Messages are queued and processed in order, decoupling object interactions.

### 10. Pathfinding (CPathSearch)

**CPathSearch** — A* pathfinding:
- Grid-based (32x16 pixels per tile)
- Obstacle avoidance (doors, walls, creatures)
- Personal space considerations
- Smoothing for natural movement

## Data Flow

### Game Loop (per tick)

```
1. Input (keyboard/mouse) → CChitin
2. Update AI scripts → CGameAIBase
3. Process actions → CGameSprite
4. Update effects → CGameEffectList
5. Recalculate stats → CDerivedStats
6. Process messages → CMessage handler
7. Render world → CInfinity
8. Render UI → CScreenWorld
9. Swap buffers → CVideo
```

### Area Transition

```
1. Save current area state
2. Unload area resources
3. Load new area (.ARE, .WED, .TIS)
4. Spawn creatures from spawn points
5. Execute area script
6. Fade in
```

## Notable Design Decisions

### 1. Flat File Layout

All source files are in `src/` (no subdirectories by subsystem). This matches the original development structure.

### 2. MFC Dependency

The engine uses Microsoft Foundation Classes (MFC):
- `CString` for strings
- `CList`, `CMap` for collections
- `CWnd` for window management

This makes cross-platform porting challenging but preserves the original architecture.

### 3. Address Comments

Every function has a comment with its binary address:
```cpp
// 0x44C8B0
CGameAIBase::CGameAIBase() { ... }
```

This enables cross-referencing with Ghidra and the original binary.

### 4. Hungarian Notation

Member variables use MFC Hungarian prefixes:
```cpp
int m_nHitPoints;        // m_ = member, n = int
BOOL m_bInCombat;        // m_ = member, b = boolean
CResRef* m_pResRef;      // m_ = member, p = pointer
```

## Reverse Engineering Approach

1. **Binary Analysis** — Ghidra decompiles `IWD2.exe`
2. **Cross-Reference** — Match functions with existing source
3. **Pattern Matching** — Use IE game similarities (BG2EE, IWDEE)
4. **Manual Review** — Decompile complex functions, understand logic
5. **Implementation** — Write C++ matching decompiled behavior
6. **Testing** — Build and verify against original game

## Known Gaps

### World Screen (CScreenWorld)

The main gameplay screen is not yet functional. Blockers:
- `CScreenWorld::Initialize()` — incomplete
- Area rendering integration
- Party movement on world map
- Combat initialization

### Missing Functions

Approximately **1,800 functions** are missing from the source:
- 222 `sub_` placeholders remain unnamed
- 999 `field_` members need semantic names
- ~100 `TODO: Incomplete` stubs need implementation

### Platform Limitations

- Windows-only (Win32, MFC, DirectDraw)
- 32-bpp windowed mode only
- No SDL/OpenGL abstraction yet

## Future Directions

1. **Complete the World Screen** — Priority #1
2. **Cross-platform port** — SDL2 + OpenGL/Vulkan
3. **Mod support** — IEex integration
4. **Modern features** — Widescreen, UI scaling, cloud saves
5. **Documentation** — Complete API docs, tutorials

## References

- [Infinity Engine Wiki](http://iesdp.gibberlings3.net/) — Technical documentation
- [Gibberlings3 Modding Community](https://www.gibberlings3.net/) — Mods and tools
- [Near Infinity](https://github.com/NearInfinityBrowser/NearInfinity) — Game file browser

---

*This document is a living architecture guide. Contributions welcome!*
