# Contributing to Icewind Dale 2 Reverse Engineering

Thank you for your interest! This guide will help you get started contributing to the project.

## Quick Start

1. Fork the repository
2. Clone your fork
3. Build the project (see [README.md](README.md))
4. Run the game and see what works
5. Pick an issue or area to contribute

## Ways to Contribute

### Reverse Engineering (Most Needed)

Help identify and name unrecognized functions:

1. **Find a `sub_` function** in the source
2. **Look it up in Ghidra** — decompile to understand behavior
3. **Give it a real name** — PascalCase, descriptive
4. **Update source files** — rename declarations and definitions

Example:
```cpp
// Before
int CGameAIBase::sub_45B6D0() {
    return nfield_58C;
}

// After
// 0x45B6D0
int CGameAIBase::GetAITarget() {
    return nfield_58C;
}
```

### Implement Stubs

Find `// TODO: Incomplete` markers and implement the function:

```cpp
// Before
void CAIGroup::GroupAction(CAIAction action, BOOL override, CAIAction* leaderAction) {
    // TODO: Incomplete.
}
```

1. Check the binary in Ghidra for the actual implementation
2. Translate the decompiled pseudocode to C++
3. Match the existing coding style
4. Test compilation

### Document Fields

Name `field_` members with meaningful Hungarian notation:

```cpp
// Before
/* 058C */ int nfield_58C;

// After
/* 058C */ int m_nAITargetId;  // m_ = member, n = int
```

### Testing

Build the project and test:
- Does it compile?
- Does it link?
- Does it run?
- What UI screens work?
- What breaks?

Report findings in GitHub Issues.

## Development Workflow

### Finding Something to Do

Check these files for work items:
- `grep -r "TODO: Incomplete" src/` — Unimplemented functions
- `grep -r "sub_" src/ | head -50` — Unnamed functions
- `grep -r "field_" src/ | head -50` — Unnamed fields

### Making Changes

1. **Create a branch** for your work:
   ```bash
   git checkout -b feature/name-functions
   ```

2. **Make minimal changes** — One class or subsystem at a time

3. **Follow naming conventions**:
   - PascalCase for classes and functions
   - `m_` prefix for members
   - Address comments: `// 0xNNNNNN`

4. **Test compilation** before committing

5. **Commit with clear messages**:
   ```
   rename: CGameSprite movement functions
   
   Named sub_453160, sub_453170, sub_4531B0:
   - SetBHiding / GetBHiding — hiding state control
   - SetField562C — equipment field setter
   ```

### Submitting Pull Requests

1. Fork the repo
2. Create a feature branch
3. Make focused changes
4. Push to your fork
5. Open a pull request with description

## Coding Standards

### C++ Style

- **Indentation**: 4 spaces (no tabs)
- **Brackets**: Allman style
  ```cpp
  if (condition) {
      // code
  }
  ```
- **Naming**: PascalCase for types/functions, camelCase for locals
- **Comments**: `//` style, above the code they describe

### Hungarian Notation

We use MFC-style Hungarian notation:
- `m_` — class member
- `n` — int/short
- `b` — boolean/BYTE
- `p` — pointer
- `dw` — DWORD
- `w` — WORD
- `sz` — null-terminated string
- `str` — CString
- `l` — LONG
- `f` — float
- `h` — handle/pointer to struct

Example:
```cpp
class CGameSprite {
    int m_nHitPoints;           // HP
    BOOL m_bInCombat;           // TRUE if fighting
    CResRef* m_pResRef;         // Animation resource
    CDerivedStats* m_pDerived;  // Calculated stats
    POSITION m_posList;         // List position
};
```

### Address Comments

Every function should have its binary address:
```cpp
// 0x44C8B0
CGameAIBase::CGameAIBase() {
    // ...
}
```

## Tools

### Required
- **Visual Studio 2019+** — Build environment
- **Git** — Version control

### Recommended
- **Ghidra** — Reverse engineering
  - GhidraMCP plugin for automated interaction
  - Our Ghidra project on Codeberg
- **Near Infinity** — Browse original game files
- **Process Monitor** — Debug file access issues

### Optional
- **IDA Pro** — Alternative disassembler
- **x64dbg** — Runtime debugging

## Understanding the Code

### Key Classes to Know

Start with these in order of complexity:

1. **CGameObject** — Base for all world objects
2. **CGameAIBase** — AI, actions, scripts
3. **CGameSprite** — Characters (huge but central)
4. **CInfGame** — Game state singleton
5. **CInfinity** — World renderer

### Reading Decompiled Code

Ghidra pseudocode looks like this:
```c
int FUN_0045b6d0(int param_1) {
    return *(int *)(param_1 + 0x58c);
}
```

This translates to:
```cpp
// 0x45B6D0
int CGameAIBase::GetAITarget() {
    return m_nAITargetId;  // at offset 0x58C
}
```

### Finding Function Addresses

The `sub_` name encodes the address:
- `sub_45B6D0` → address `0x0045B6D0`
- In Ghidra: Search → For Functions → `FUN_0045B6D0`

## Getting Help

- **GitHub Discussions** — Questions about the code
- **GitHub Issues** — Bug reports and feature requests
- **Infinity Engine Discords** — General IE modding community
- **Gibberlings3 Forums** — Technical IE discussions

## License

By contributing, you agree that your code will be under the [Sustainable Use License](LICENSE.md).

## Acknowledgments

Contributors will be credited in the project history and future release notes.

---

*Happy reverse engineering! The cold winds of Icewind Dale await.*
