# Build and run on native Windows

The Windows workflow runs the compiler, game and analysis tools on the same
machine. Linux + Windows VM remains supported through `scripts/vm.sh`.
For parity, callers/callees, Ghidra, Frida and setup reproduction, see
[Windows native workflow](docs/windows-native-workflow.md). Read `CLAUDE.md`
explicitly if `AGENTS.md` appears as a one-line symlink target on Windows.

## Prerequisites

- Visual Studio 2019 or its Build Tools, with **MSVC v142 C++ x64/x86 tools**,
  **ATL v142**, **MFC v142**, and a Windows SDK. ATL/MFC alone does not install
  the compiler. The build target is **Win32/x86**.
- CMake, Git and Python; the analysis workflow uses `.venv-reagent`.
- Original game assets, currently `C:\Games\Icewind Dale 2`.

Clone with the reference/tool submodules when setting up another checkout:

```powershell
git clone --recurse-submodules https://github.com/WillScarlettOhara/iwd2-re.git
Set-Location iwd2-re
```

For an existing checkout, run `git submodule update --init --recursive`.

## Build

From the repository root, with the venv from the workflow guide installed:

```powershell
$py = '.\.venv-reagent\Scripts\python.exe'
& $py scripts/native.py build
```

This configures VS2019/v142, builds `build/Debug/iwd2-re.exe`, and performs a
clean rebuild when header state changes. Full output is in `tmp_native_build.log`.
It never stops a running game. If the linker reports a locked executable, close
that instance or run `& $py scripts/native.py kill ours`, then rebuild.

The equivalent initial CMake commands are:

```powershell
cmake -S . -B build -G 'Visual Studio 16 2019' -A Win32 -T v142
cmake --build build --config Debug --parallel 4
```

Use a fresh build directory if its CMake cache selects a different generator or
architecture. `IWD2_BUILD_DIR` and `IWD2_CMAKE` override local build/tool paths.

## Run and observe

```powershell
$env:IWD2_GAME_DIR = 'C:\Games\Icewind Dale 2'
& $py scripts/native.py run --new-game --party 0
# Or auto-load an existing visible slot:
& $py scripts/native.py run 3
# Guard a loaded game and require a function to execute:
& $py scripts/native.py smoke 3 60 --hit 'CGameSprite::Render'
```

`run` keeps the game open; `smoke` closes its process after observation. Use
`--save NAME` for an exact MPSave directory or `--new-game --party 0` when testing
creation. Both commands launch the build with the game directory as its working
directory; there is no need to copy it into the installation. Native build and
launch also copy the installation's DirectDraw compatibility DLLs beside the
executable when present. This matters on Windows: the game CWD alone does not
make its `ddraw.dll` take precedence over the system DLL. Existing different
DLLs are preserved and reported as a conflict. `IWD2_SKIP_RUNTIME_COMPANIONS=1`
opts out when deliberately testing another graphics runtime.

To open the normal menus without AutoLoad, double-click `run-game.bat`, or run:

```powershell
& $py scripts/native.py menu
```

`RESULT: CLEAN` checks loading/crashes over the observation interval. It does
not establish behavioral parity; use `--hit`, an AutoUI scenario and matched
original/recovered observations for the affected path.

For startup failures, first check the game CWD contains `chitin.key`, `data/`
and the remaining installed assets. Debug runtime DLLs come from the Visual
Studio C++ installation; installing only a release redistributable does not
provide the Debug runtime.
