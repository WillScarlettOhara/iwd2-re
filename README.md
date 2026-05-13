# Icewind Dale 2 - Reverse Engineered

> **Recover the lost source code.** An open-source recreation of the Icewind Dale II engine, built via community-driven reverse engineering.

---

## What is this?

This repository contains **reverse-engineered C++ source code** for **Icewind Dale II** (2002), the final Infinity Engine game from BioWare/Black Isle Studios.

Original source code was never released. This project reconstructs it by analyzing the compiled binary and translating machine code back into readable, modern C++.

**Goal**: Create a fully working, open-source engine that can run Icewind Dale II, then extend it with community improvements.

---

## Project Status

> **Not playable yet** — but getting closer.

| Metric | Progress | Details |
|--------|----------|---------|
| **Functions** | ~80% (~8,200 / ~10,000) | Most core systems identified |
| **Code** | ~53% (~2M / ~3.8M lines) | Decompiled and organized |
| **Named Functions** | Ongoing | ~200 `sub_` remaining |
| **Named Fields** | Ongoing | ~640 `field_` remaining |
| **Main Menu** | **Working** | Boots directly to CScreenConnection; mouse cursor visible and clickable |
| **UI Screens** | Working | Options, Keymaps, Single Player, Party Select, Character Editor |
| **World Screen** | **Not working** | Current blocker for gameplay |

**Current milestone**: Stabilize the World Screen transition so the main gameplay loop can start.

---

## Architecture

The Infinity Engine is split into several subsystems:

| Subsystem | Files | Description |
|-----------|-------|-------------|
| **Core Engine** | `CChitin`, `CBaldurChitin`, `CInfinity` | App framework, message loop, renderer |
| **Game State** | `CInfGame`, `CGameSave`, `CGameArea` | Party, areas, saves, journal, world map |
| **Game Objects** | `CGameSprite`, `CGameDoor`, `CGameTrigger`, `CGameAIBase` | All interactive world entities |
| **AI/Scripting** | `CAIScript`, `CAIAction`, `CAITrigger`, `CAIObjectType` | Script parser and execution |
| **UI** | `CScreenWorld`, `CScreenCharacter`, `CScreenInventory`, ... | All game screens |
| **Resources** | `CRes`, `CDimm`, `CResRef` | Asset loading (.ARE, .CRE, .ITM, etc.) |
| **Messages** | `CMessage` | Inter-object communication system |
| **Effects** | `CGameEffect`, `CDerivedStats` | Spells, buffs, modifiers |
| **Network** | `CNetwork` | Multiplayer support |
| **Audio** | `CSoundMixer`, `CSound` | Sound effects and music |

---

## Building

### Requirements

- **Visual Studio 2019** (or newer)
- **Windows 10/11**
- **C++17**
- **MFC** (Microsoft Foundation Classes)
- **DirectDraw**
- Original game files from [GOG](https://www.gog.com/game/icewind_dale_2) or Steam

### Quick Start

```bash
# Clone with all reference submodules
git clone --recursive git@github.com:WillScarlettOhara/iwd2-re.git
cd iwd2-re

# Restore bundled data (PDB extracts + Ghidra projects)
powershell -File data/restore.ps1

# Build (Win32/Debug or Release)
cmake --build build --config Debug
```

> Only **32-bpp windowed mode** is currently implemented.

---

## Contributing

We need help! Here's how you can contribute:

### 1. Reverse Engineering
- Decompile functions using [Ghidra](https://ghidra-sre.org/)
- Name `sub_` placeholders with meaningful names
- Document `field_` members
- Cross-reference with other Infinity Engine games (BG2EE, IWDEE)

### 2. Implement Stubs
- Find `// TODO: Incomplete` markers
- Implement missing function bodies based on decompiled pseudocode
- Focus on World Screen blockers

### 3. Testing
- Build and test the game
- Report what works/breaks
- Verify fixes against original game

### 4. Documentation
- Add class header comments
- Write architecture docs
- Help newcomers understand the code

### Naming Conventions

We use **MFC Hungarian notation**:
- `m_` — class members
- `n` — integer
- `p` — pointer
- `b` — boolean
- `w` — WORD/short
- `dw` — DWORD

Address comments mark original binary locations:
```cpp
// 0x7D14F0
int CGameSprite::GetDerivedStats() {
    return m_pDerivedStats;
}
```

---

## Tools & Reference Data

Everything needed for RE work is bundled in this repo or linked as submodules.

### Bundled Data (`data/`)
| Archive | Contents | Uncompressed |
|---------|----------|-------------|
| `data/pdb/bg2_pdb_extracted.zip` | BG2EE PDB types, symbols, globals | 126 MB |
| `data/ghidra/IWD2_rep.zip` | IWD2 Ghidra project repository | 301 MB |
| `data/ghidra/BG2EE_rep.zip` | BG2EE Ghidra project repository | 227 MB |

Run `data/restore.ps1` after clone to extract to `C:\projects\` and `C:\ghidra_projects\`.

### Git Submodules (`refs/`)
| Submodule | Source | Use |
|-----------|--------|-----|
| `refs/gemrb` | github.com/gemrb/gemrb | CGameSprite→Actor, CInfGame→Game mappings |
| `refs/NearInfinity` | github.com/NearInfinityBrowser/NearInfinity | IWD2 file formats (.CRE, .ARE, .ITM) |
| `refs/iesdp` | github.com/Gibberlings3/iesdp | IWD2 effects (353 opcodes), STATS.IDS |

### External Tools
| Tool | Purpose | Link |
|------|---------|------|
| **Ghidra 12.0.4** | Reverse engineering framework | [ghidra-sre.org](https://ghidra-sre.org/) |
| **GhidraSQL** | SQL interface to Ghidra DB | bundled with LibGhidraHost |
| **Visual Studio 2019** | Build compiler (Win32, C++17, MFC) | — |

---

## Related Projects

- [alexbatalov/iwd2-re](https://github.com/alexbatalov/iwd2-re) — Upstream project by Alexander Batalov
- **GemRB** — Open-source Infinity Engine implementation (see `refs/gemrb`)
- **NearInfinity** — Infinity Engine file browser (see `refs/NearInfinity`)
- **IESDP** — Infinity Engine Structures Description Project (see `refs/iesdp`)
- **BG2EE** / **IWDEE** — Beamdog's official Enhanced Editions (PDB symbols in `data/pdb/`)

## Documentation

- **[AGENTS.md](AGENTS.md)** — Full workflow guide: GhidraSQL queries, rename strategy, build safety rules
- **[decomp_ref/](decomp_ref/)** — Memory nodes per subsystem with field mappings and discoveries

---

## License

This source code is available under the [**Sustainable Use License**](LICENSE.md).

> **Note**: This project does NOT distribute game assets. You must own a legitimate copy of Icewind Dale II.

---

## Acknowledgments

- **Alexander Batalov** — Original reverse engineering work
- **Beamdog** — Enhanced Editions and keeping IE alive
- **Ghidra Team (NSA)** — For the incredible reverse engineering tool
- **Infinity Engine Community** — Decades of modding and documentation

---

## Community

- GitHub Discussions: [github.com/WillScarlettOhara/iwd2-re/discussions](https://github.com/WillScarlettOhara/iwd2-re/discussions)
- Infinity Engine Discord communities
- r/icewinddale — Reddit community

---

> *"The winds of Icewind Dale carry whispers of a lost codebase... recovered, one function at a time."*
