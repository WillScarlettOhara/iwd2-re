# Native Windows recovery workflow

Use the same sequence as on Linux: source lookup → binary callers/callees and
context → assembly/runtime evidence → edit → parity/layout checks → build →
exercise the affected game path. `scripts/arc.py verify` coordinates the gates.
Native Windows uses `scripts/native.py`; Linux keeps `scripts/vm.sh` and the
configured `win11vm`. The recovery rules remain in [CLAUDE.md](../CLAUDE.md).

## Session and tools

Run from the repository root (currently `C:\Users\leilu\Repo\iwd2-re`):

```powershell
$repo = (Get-Location).Path
$py = Join-Path $repo '.venv-reagent/Scripts/python.exe'
$graph = Join-Path $repo '.venv-reagent/Scripts/code-review-graph.exe'
$reagent = Join-Path $repo '.venv-reagent/Scripts/re-agent.exe'
$env:IWD2_GAME_DIR = 'C:\Games\Icewind Dale 2'
$env:CRG_HOME = Join-Path $repo '.code-review-graph/registry'
$env:PYTHONUTF8 = '1'
```

For a fresh checkout, install the [VS2019/v142 prerequisites](../BUILD_WINDOWS.md),
initialize submodules, and create the venv. These package versions were used for
this migration; the bridge and re-agent stay tied to the vendored code:

```powershell
py -3 -m venv .venv-reagent
& $py -m pip install -e './vendor/ghidra-ai-bridge[headless]' -e ./vendor/auto-re-agent
& $py -m pip install pefile==2024.8.26 capstone==5.0.9 minidump==0.0.24 pyghidra==3.1.0 frida==17.17.0 frida-tools==14.10.4 code-review-graph==2.3.8 websockets==13.1
& $py -m pip check
```

Do not reuse a Linux virtualenv on Windows: executables are under `Scripts/`
instead of `bin/`. Native helpers discover local Java and cppcheck without
changing global settings. Overrides: `IWD2_JAVA_HOME`/`JAVA_HOME`, `IWD2_CPPCHECK`,
`IWD2_CMAKE`, `IWD2_BUILD_DIR`. This installation uses Ghidra 12.1.3 under
`.tools/ghidra_12.1.3_PUBLIC` and cppcheck 2.21 at
`.tools/cppcheck/PFiles/Cppcheck/cppcheck.exe`.

For direct PyGhidra calls, set `JAVA_HOME` to the installed compatible JDK
(this machine uses `C:\Program Files\Java\jdk-25.0.2`), and set:

```powershell
$env:GHIDRA_INSTALL_DIR = Join-Path $repo '.tools/ghidra_12.1.3_PUBLIC'
```

## Canonical binary, snapshot and host configuration

Static analysis defaults to `.bin/IWD2.exe`, MD5
`25cb3d8aa2ec648e94e328d8c467973e` (ImageBase `0x400000`, no ASLR).
`IWD2_EXE` overrides that path. Check it before comparing offsets:

```powershell
& $py scripts/sym.py exe
Get-FileHash .bin/IWD2.exe -Algorithm MD5
```

Keep the canonical binary separate from the game installation's executable;
mods can change the installed image. Original runtime traces default to
`IWD2.exe` in `IWD2_GAME_DIR`; verify its hash for matched comparisons.

Ghidra's analyzed project ships as the `IWD2_rep.zip` asset of the rolling
[`ghidra-snapshot` release](https://github.com/WillScarlettOhara/iwd2-re/releases/tag/ghidra-snapshot).
To restore into a **new local directory**, with Ghidra closed:

```powershell
gh release download ghidra-snapshot --repo WillScarlettOhara/iwd2-re --pattern IWD2_rep.zip --dir data/ghidra
Expand-Archive -LiteralPath data/ghidra/IWD2_rep.zip -DestinationPath ghidra-projects
```

The downloaded archive used here contains `IWD2/IWD2.gpr` and `IWD2/IWD2.rep`,
so the configured project directory is `ghidra-projects/IWD2`. Inspect the
archive layout when replacing it with a newer snapshot. Do not overwrite a
project containing local analysis. Avoid `.ghidra-projects`: Ghidra rejects
dot-containing project paths on this setup.

A copied Linux snapshot can retain the previous owner's name. For this private
downloaded copy only, migrate with Ghidra's supported `resetOwner` constructor,
with no GUI/headless process holding it. Save this as a temporary Python script
and run it with `$py` after setting the Java/Ghidra environment:

```python
from pathlib import Path
import pyghidra

root = Path("ghidra-projects/IWD2").resolve()
assert (root / "IWD2.gpr").is_file()
pyghidra.start()
from ghidra.framework.model import ProjectLocator
from ghidra.framework.data import DefaultProjectData

# The final argument requests owner reset for this imported private copy.
project = DefaultProjectData(ProjectLocator(str(root), "IWD2"), True, True)
try:
    print("Imported snapshot owner:", project.getOwner())
finally:
    project.close()
```

Use this migration only on the newly imported local snapshot, never to take
over a locked/live project. It preserves the program database and changes the
local project's recorded owner.

Create the following two **gitignored** files at the repository root. Replace
every `<REPO>` with the checkout's absolute path using forward slashes; YAML
does not expand this placeholder.

`ghidra-bridge.host.yaml`:

```yaml
ghidra:
  install_dir: <REPO>/.tools/ghidra_12.1.3_PUBLIC
  project_dir: <REPO>/ghidra-projects/IWD2
  project_name: IWD2
  program_name: IWD2.exe
paths:
  export_dir: <REPO>/.ghidra-exports
  address_map: <REPO>/.ghidra-exports/address_map.json
source:
  root: <REPO>/src
  hook_patterns: []
  stub_patterns: []
  file_extensions: [".cpp", ".h", ".hpp"]
binary:
  code_range_min: "0x00401000"
  code_range_max: "0x00800000"
```

`re-agent.host.yaml` (static checks need no LLM key):

```yaml
project_profile:
  source_root: <REPO>/src
  source_extensions: [".cpp", ".h", ".hpp"]
  hooks_csv: <REPO>/.ghidra-exports/hooks.csv
  hook_patterns: []
  stub_patterns: []
  stub_markers: ["TODO: Incomplete"]
backend:
  type: ghidra-bridge
  cli_path: <REPO>/scripts/ghidra-bridge-host.cmd
  timeout_s: 180
parity:
  enabled: true
  call_count_warn_diff: 3
  inline_wrapper_autoskip: false
  cache_dir: .cache/re-agent-parity
orchestrator:
  objective_verifier_enabled: true
  objective_call_count_tolerance: 3
  objective_control_flow_tolerance: 2
```

The `.cmd` wrapper invokes `ghidra_bridge_host.py`, selects the local JDK and
always supplies `ghidra-bridge.host.yaml`. On Linux use the existing
`scripts/ghidra-bridge-host` wrapper and Linux project/install paths. Optional
batch LLM work can use a key in the ignored `.env`; it is independent of static
parity, graph and runtime checks.

## Export, context and parity

Build the address map from source annotations before exporting. **Never use
`ghidra-bridge build-map`**: its hook-macro parser does not match this source.

```powershell
& $py scripts/reagent_address_map.py --out .ghidra-exports/address_map.json
Expand-Archive -LiteralPath data/pdb/bg2_pdb_extracted.zip -DestinationPath data/pdb/extracted
& $py scripts/bg2_find.py --build
& $py scripts/reagent_bg2_pdb.py
& ./scripts/ghidra-bridge-host.cmd export all
& $py scripts/src_find.py 'CGameAIBase::ApplyTriggers'
& $py scripts/fn_digest.py 0xADDR
& ./scripts/ghidra-bridge-host.cmd xrefs-to 0xADDR
& ./scripts/ghidra-bridge-host.cmd xrefs-from 0xADDR
& $py scripts/reagent_assemble_context.py --address 0xADDR --out tmp_ctx.md
& $reagent --config re-agent.host.yaml parity --address 0xADDR
```

Replace `0xADDR` with the address returned by source lookup. The BG2 helper can
also read the ZIP directly; extracting it supplies `reagent_bg2_pdb.py`'s input.
BG2 names are hints; BG2 offsets are not IWD2 offsets. If a valid virtual method
has no exported function, verify its address, refresh the address map, then run
`export create-functions`. Close Ghidra before headless export or parity steps
that request assembly. An export includes compiler helper functions; its raw
function count is not the number of game functions left to recover.

Parity resolves an adjacent `// 0xADDR` marker before a method name. This makes
overloaded methods address-exact; a name-only fallback is used for sources that
have no marker. A call-count YELLOW remains advisory because compiler-generated
and inlined calls can differ, while RED findings and behavioral claims still
need assembly or runtime evidence.

## Source callers/callees and MCP

The source graph is separate from Ghidra's binary call graph. Build each
repository independently; no embedding download is needed:

```powershell
& $graph build --repo .
& $graph build --repo refs/gemrb
& $graph register . --alias iwd2-re
& $graph register refs/gemrb --alias gemrb
& $graph query callers_of ApplyTriggers --repo .
& $graph query callees_of ApplyTriggers --repo .
& $graph query callers_of ExecuteScript --repo refs/gemrb
& $graph impact --files src/CGameAIBase.cpp --repo .
& $graph flows --limit 5 --repo .
& $graph update --repo .
```

Keep `CRG_HOME` set to the local registry above, including in the MCP server
environment. C++ declarations/definitions and overrides may share a name:
when a query reports `ambiguous`, rerun with the returned definition's
`qualified_name`. For example, `ApplyTriggers` in `CGameAIBase.cpp` resolves to
`<REPO>/src/CGameAIBase.cpp::CGameAIBase.ApplyTriggers()`. Unresolved calls mean
the graph can miss impact paths; binary xrefs/assembly remain the parity oracle.
Without optional igraph, community detection falls back to grouping by file.

This checkout has ignored local `.codex/config.toml` and `.mcp.json` files for
Codex and Claude Code. Both start the venv's `code-review-graph.exe` with
`serve --repo <absolute-repo> --auto-watch` and set `CRG_HOME` plus `PYTHONUTF8=1`. Regenerate
their absolute paths when moving the checkout. Codex loads project MCP config
for trusted projects: open this repository in a new project session to use it;
an already running projectless session can use the CLI above. See the
[official MCP configuration documentation](https://learn.chatgpt.com/docs/extend/mcp?surface=cli).
No global user configuration is needed. Read `CLAUDE.md` explicitly when
`AGENTS.md` is checked out as a plain one-line symlink target on Windows.

## Build, exercise and inspect evidence

```powershell
& $py scripts/native.py build
& $py scripts/native.py menu
& $py scripts/native.py run --new-game --party 0
& $py scripts/native.py smoke --new-game --party 0 --hold 60 --ui scripts/scenarios/newgame-reload.txt
& $py scripts/native.py smoke 3 60 --hit 'CGameSprite::Render'
& $py scripts/native.py trace --hooks scripts/probes/hooks.json --load-slot 3 --hit NAME
& $py scripts/native.py status
& $py scripts/native.py log 'REGEX' -n 50
& $py scripts/arc.py check
& $py scripts/arc.py verify 0xADDR --new-game --party 0 --hold 60 --require exe,vm
& $py scripts/arc.py batch 0xADDR 0xADDR --new-game --party 0 --hold 60 `
    --ui scripts/scenarios/newgame-reload.txt --require exe,vm
```

Choose an existing hook table and its hook name for `trace`; `--load-slot -1`
leaves the original at its menus. Prefer `--save NAME` over a changing slot
number for repeatable recovered-build tests. Native smoke arms the observer
before process resume, waits for world activation, observes for `--hold`, checks
AutoUI completion and optional `--hit`/`--expect`, then closes its process.
AutoUI scripts should start with `waitgo`, then wait for the relevant screen.

For typed debug instrumentation, check/create
`$env:IWD2_GAME_DIR\iwd2-re-debug.enabled`. Logs append; `native.py run/smoke`
clear the debug log before launch. Use `native.py clearlog` before other fresh
captures. Evidence lives in `tmp_native_load.txt`, `tmp_native_status.txt`,
`tmp_smoke_guard.log`, `tmp_ui_result.jsonl`, and the game's `iwd2-re-debug.log`.
Original trace defaults to `tmp_orig_trace.jsonl` plus its `.status` file.

Native build/launch stage the installed game's `ddraw.dll` and its optional
`libwine.dll`/`wined3d.dll` companions beside the build. The executable directory
must contain the wrapper for it to precede Windows' own DirectDraw. Identical
files are retained; a different existing file causes an explicit conflict,
without overwriting it. For a deliberate alternative runtime, set
`IWD2_SKIP_RUNTIME_COMPANIONS=1`. During this migration, matching the installed
DDrawCompat changed sampled prologue rendering from 7–10 to 30–31 frames/sec.

Install the source pre-commit gate with `& $py scripts/arc.py hooks --install`.
The graph server's watcher updates source context during MCP sessions; for CLI
sessions, run `& $graph update --repo .` after edits.

`arc` selects the native adapter automatically on Windows. Its legacy
`--require exe,vm` means the canonical binary and available runtime adapter;
it does not connect to a VM on Windows. `--static` skips build/runtime,
`--no-parity` skips parity, and `.arc/<run>/` stores full gate output accessible
through `arc.py explain`. A skipped gate is not completed validation.

Use `arc batch` for 2–10 functions reached by the same save and AutoUI scenario.
It preserves the static result of each address in `targets[]`, deduplicates
class-wide work, compiles once, and launches one smoke. Its default hit list is
all resolved source symbols; the smoke passes only when every symbol executes.
Repeat `--hit` to select representative hooks when the scenario cannot exercise
all targets. Windows and the Linux/VM adapter share the same comma-separated
multi-symbol counter contract.

Smoke exit 0 is CLEAN, 1 is failure, and 2 means unavailable/unproven (including
harness errors or a symbol never hit). Static GREEN, successful export, a built
graph, and a bare crash-free interval each prove different things. Claim a game
fix only after exercising and observing its affected behavior.

CRE V2.2 saves must set `m_effectVersion` to `1` when writing 0x108-byte
`CGameEffectBase` records. A save made by the rebuilt executable should also be
loaded by the original executable as a cross-compatibility gate. Test saves
created before this field was restored need to be regenerated or migrated from
a backup; the original loader otherwise reads the `EFF V2.0` signature as a
legacy effect id and asserts with decimal id `17989`.
