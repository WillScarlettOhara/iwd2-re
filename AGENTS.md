# AGENTS.md — iwd2-re Project

Reverse engineering of Icewind Dale 2 — recover the lost source code.

## Repo
- **Upstream**: https://github.com/alexbatalov/iwd2-re
- **Fork**: https://github.com/WillScarlettOhara/iwd2-re

## Current Status
- Functions: 80% (~8,200 / ~10,000)
- Code: 53% (~2M / ~3.8M lines)
- Game: NOT playable (only UI screens work)

## Tooling
- Ghidra 12.0.4 + GhidraMCP headless (Docker, port 8089)
- Binary: /home/wills/Games/Heroic/Icewind Dale 2/IWD2.exe (28,362 functions identified)
- Auth token: `iwd2ee-re-tools-2026`
- Source reference: /home/wills/projects/IWD2-RE/iwd2-re/

## Directory Structure
- `src/` — Flat layout, ~202 .cpp + ~209 .h, organized by naming convention
- `src/music/` — Audio subsystem (Miles Sound System)
- `third_party/` — zlib, DirectPlay headers
- Build: Visual Studio 2019 Win32, C++17, MFC, DirectDraw

## Key Commands
```bash
# Ghidra MCP Docker
cd /home/wills/projects/IWD2-RE/ghidra-mcp/docker
docker compose up -d
docker compose down -v   # clean reset

# Test MCP
curl -H "Authorization: Bearer iwd2ee-re-tools-2026" http://127.0.0.1:8089/check_connection

# Load binary
curl -X POST -H "Authorization: Bearer iwd2ee-re-tools-2026" -H "Content-Type: application/json" -d '{"file":"/iwd2/IWD2.exe"}' http://127.0.0.1:8089/load_program

# Run analysis
curl -X POST -H "Authorization: Bearer iwd2ee-re-tools-2026" http://127.0.0.1:8089/run_analysis

# List functions
curl -H "Authorization: Bearer iwd2ee-re-tools-2026" "http://127.0.0.1:8089/list_functions?limit=100"
```

## Naming Conventions
- Address comments: `// 0x7D14F0` above every function/variable
- Class offsets: `/* 0044 */ RESREF groundIcon;`
- Unnamed functions: `sub_NNNNNN` (address-based placeholder)
- Unnamed fields: `field_X` (offset-based placeholder)
- MFC Hungarian: `m_` members, `n` ints, `p` pointers

## Progress Tracking
- Undocumented `sub_` functions: ~243
- Unnamed `field_` members: ~974
- TODO markers: ~464
- FIXME markers: ~427
- Incomplete markers: ~383

## Plan
1. Discover missing ~1,800 functions (Ghidra cross-referencing)
2. Name/document anonymous code (sub_*, field_*)
3. Implement incomplete code (~1,500 markers)
4. Make game playable (World Screen)
5. Modernize (cross-platform, SDL2/OpenGL)
6. Publish
