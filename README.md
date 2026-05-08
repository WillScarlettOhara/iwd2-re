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
| **Functions** | ~82% (~8,221 / ~10,000) | Most core systems identified |
| **Code** | ~53% (~2.0M / ~3.8M lines) | Decompiled and organized |
| **UI Screens** | Working | Start, Options, Keymaps, Party Select, Character Editor |
| **World Screen** | **Not working** | This is the current blocker for gameplay |
| **Named Functions** | +21 recent | Removed `sub_` placeholders |
| **Docs** | In progress | 28 core classes documented |

**Current milestone**: Implement missing functions to reach the World Screen (main gameplay).

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
# Clone the repository
git clone https://github.com/WillScarlettOhara/iwd2-re.git
cd iwd2-re

# Open in Visual Studio
start iwd2-re.sln

# Build (Win32/Release)
# Install original game to default path for testing
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

## Tools We Use

| Tool | Purpose | Link |
|------|---------|------|
| **Ghidra** | Reverse engineering framework | [ghidra-sre.org](https://ghidra-sre.org/) |
| **GhidraMCP** | MCP bridge for automated analysis | [github.com/...](...) |
| **IEex** | Community framework for IE games | [GitHub](https://github.com/) |
| **IDA Pro** | Alternative disassembler (optional) | [hex-rays.com](https://hex-rays.com/) |

---

## Related Projects

- [alexbatalov/iwd2-re](https://github.com/alexbatalov/iwd2-re) — Upstream project by Alexander Batalov
- **Infinity Engine Enhanced Editions** — Beamdog's official remasters
- **GemRB** — Open-source Infinity Engine implementation (alternative approach)
- **BG2EE** / **IWDEE** — Other IE games with Enhanced Edition support

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
