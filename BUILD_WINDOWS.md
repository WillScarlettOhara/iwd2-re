# Build & Run IWD2-RE on Windows 11

## Prerequisites

1. **Visual Studio 2022 Community** (free)
    - Download: https://visualstudio.microsoft.com/downloads/
    - During install, check **"Desktop development with C++"**
    - In "Individual components" tab, find and check **"MFC for latest v143 build tools"** (Microsoft Foundation Classes for C++)

2. **Git** — https://git-scm.com/

## Build

Open **Developer PowerShell** (Start → "Developer PowerShell for VS 2022"):

```powershell
# Clone
git clone https://github.com/WillScarlettOhara/iwd2-re.git
cd iwd2-re

# Configure and build (Win32, not x64)
mkdir build; cd build
cmake .. -G "Visual Studio 16 2019" -A Win32
cmake --build . --config Debug
```

## Run

```powershell
# Copy executable to your IWD2 game folder
cp Debug/iwd2-re.exe "C:\GOG Games\Icewind Dale 2\"

# Run from game folder (needs game assets: chitin.key, data/, etc.)
cd "C:\GOG Games\Icewind Dale 2"
.\iwd2-re.exe
```

## Troubleshooting

### d3dx9_43.dll missing

```powershell
winget install Microsoft.DirectX
```

### MSVCP140.dll / VCRUNTIME140.dll missing

Install VS 2022 x86 redistributable (game is Win32, not x64):
https://aka.ms/vs/17/release/vc_redist.x86.exe

### Game crashes at startup

Make sure you're running from the IWD2 game folder. The executable needs `chitin.key`, `data/`, sound files, and other game assets to be present.
