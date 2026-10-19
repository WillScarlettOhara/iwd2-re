# Recovery technical reference

Read only the section needed for the current task. CLAUDE.md and economical-recovery.md define the current workflow; historical per-function audit recommendations below are conditional technical guidance, not a requirement to run every check or create new probes.

## Build & run

Native Windows, from the repository root:

```powershell
$py = '.\.venv-reagent\Scripts\python.exe'
& $py scripts/native.py build
& $py scripts/native.py run --new-game --party 0
& $py scripts/native.py smoke 3 60 --hit 'CGameSprite::Render'
& $py scripts/native.py log 'REGEX' -n 50
& $py scripts/native.py trace --hooks scripts/probes/hooks.json --load-slot 3 --hit NAME
```

Use `--save NAME` for an exact MPSave directory, or `--new-game --party 0` when
there is no suitable save. `native.py status`, `tail`, `ps`, `clearlog` and
`kill ours` cover local diagnostics. `build` never kills a game; `run`/`smoke`
replace only this checkout's executable. `smoke` closes the process it launched.

Linux host → VM over SSH remains available:

```bash
scripts/vm.sh build [--run]   # remote_build.sh wrapped: prints ONLY errors/warnings + status (full log tmp_vm_build.log)
scripts/vm.sh run [slot]      # kill OUR iwd2-re.exe + launch session 1 (default combat slot 2)
scripts/vm.sh log <regex> [-n 50] [-f path]   # Select-String VM-side (UTF-16 safe); also: tail/status/ps/pull/push
scripts/vm.sh frida script.py # ship + run as session-1 payload, ready-to-log
```
- Kill policy: `build` never kills; `run` kills our exe only. Original `IWD2.exe` is fair to kill once a trace/investigation is done — don't leave it running idle (wastes CPU/power); relaunch fresh for the next trace rather than keeping it up. Frida sessions survive builds.
- Linux/VM only: GUI over SSH = session 0 (never renders); session 1 via `vm_s1.cmd` required for render/input/Frida. Smoke timeout there = no desktop, not a crash. Native Windows launches in the current desktop session.
- Commits must compile VS2019 Win32. Rename → update `.h` + ALL `.cpp` in one commit. Build fail → stop. `git push` periodically — commits pile up unpushed on `main`.

## Truth hierarchy (the decompile is a fallible lift)

Don't invent code. Rank of ground truth:
1. **PE bytes / disassembly** (`sym.py disasm`, `gb dump-asm`) = what the CPU runs — final word on instructions, call targets, struct offsets.
2. **Frida runtime trace** = final word on dynamic facts — real `__thiscall` args, which branch runs, field/flag semantics, container contents, actual runtime types.
3. **Ghidra decompile** = best first read but FALLIBLE: guesses types, mangles `__thiscall` args, miscounts 16-vs-32-bit reads, drops/merges inlined calls, garbles SEH/reordering, invents `CONCAT`/casts. Navigate with it; verify against asm or Frida before trusting counts, args, widths, offsets, or types.

`// 0xADDR` can be stale → re-check against the binary. Address not in funcs table → check vtable DATA xref (virtual method): `scripts/sym.py findptr 0xMETHOD` → `sym.py vtable 0xBASE`.

## Ghidra access — ghidra-bridge

**ghidra-ai-bridge** (vendored `vendor/ghidra-ai-bridge`) = PyGhidra **headless export** into `.ghidra-exports/` (not a live server). Config `ghidra-bridge.host.yaml` + `re-agent.host.yaml` lives at the repo root, gitignored. On Windows the local Ghidra 12.1.3 install is `.tools/ghidra_12.1.3_PUBLIC`, the imported project is `ghidra-projects/IWD2` (avoid dot-prefixed project paths), and Java is selected through `IWD2_JAVA_HOME`/`JAVA_HOME` or local JDK discovery. Linux keeps its configured project/install paths.

re-agent's backend must use `scripts/ghidra-bridge-host.cmd` on Windows or `scripts/ghidra-bridge-host` on Linux. The wrapper supplies the host config; a direct unconfigured backend can report "Function not found" for valid addresses. Download/import and local snapshot ownership migration are documented in [the Windows workflow](windows-native-workflow.md).

**Query** (`gb` below = `scripts/ghidra-bridge-host.cmd` on Windows, `.venv-reagent/bin/ghidra-bridge --config ghidra-bridge.host.yaml` on Linux):

| Want | Cmd |
|---|---|
| Full decompile (prefer `fn_digest.py` for a look) | `gb decompile 0xADDR` |
| Callers / callees | `gb xrefs-to 0xADDR` / `gb xrefs-from 0xADDR` |
| Disassembly (boots PyGhidra ~1 min; `sym.py disasm` = instant) | `gb dump-asm 0xADDR out.asm` |
| Struct / vtable / containing fn / unrecovered list | `gb struct CClass` / `gb vtable CClass` / `gb containing 0xADDR` / `gb unimplemented CClass` |

**"Function not found" / "no export" on a VALID addr** = no fn defined there (vtable-only callee). Fix: ensure `// 0xADDR` in source (stub if new) → `python scripts/reagent_address_map.py --out .ghidra-exports/address_map.json` → `gb export create-functions` (~10 min, re-exports all).
⚠️ NEVER `gb build-map` — overwrites the map with an EMPTY one.

**Parity** = faithfulness lint (14 signals + objective verifier). Run from repo root:

```powershell
& ./.venv-reagent/Scripts/re-agent.exe --config re-agent.host.yaml parity --address 0xADDR
& ./.venv-reagent/Scripts/re-agent.exe --config re-agent.host.yaml parity --filter "CClass::" --output tmp_parity.json
```

Linux uses `.venv-reagent/bin/re-agent` with the same arguments.

GREEN/YELLOW/RED. asm signals boot PyGhidra (~1 min/call). STL-inline call-count YELLOW/RED = classic false positives.
⚠️ `re-agent reverse` = lint/oracle at best, NEVER author (regresses clean code). Don't commit its output unverified; gate behavioral claims with a Frida diff.

PE facts: the canonical `.bin/IWD2.exe` has MD5 `25cb3d8aa2ec648e94e328d8c467973e`, ImageBase `0x400000`, no ASLR. `python scripts/sym.py exe` checks the selected binary; `IWD2_EXE` overrides it. Do not assume a modified game install matches the Ghidra snapshot.

## Frida tracing

Throwaway traces to confirm field semantics live BEFORE a recover — ground truth beats guessing. Simple probes (args+fields at entries) = `frida_probe.py` hook-table JSON; complex logic (call-origin filters, list walks) = bespoke script, template `scripts/frida_formation_trace.py`, docs `docs/frida-differential-tracing.md`. **Bespoke probes go in `scripts/probes/`** — `scripts/` keeps only reusable tooling (`frida_probe`, `frida_crash_guard`, `frida_formation_trace`, `frida_hang_bt`, `frida_diff_*`).
- **OUR build's typed state → `DebugLog.h` (`Iwd2DebugLog(fmt,...)`)**: zero offset/symbol/ecx fragility (the debug-build +0xA base shift breaks original-binary Frida member reads — see `53fe6dccc766`). Check for `iwd2-re-debug.enabled` in the game CWD and create it if needed; do not assume it exists. The log is append-only: clear it before a fresh capture (`native.py run/smoke` does this). Frida is used for original-binary state/differential tracing and our build's generic crash guard/PDB hit counters.
- **Commit clean BEFORE adding `Iwd2DebugLog`.** Keeps debug isolated → strip via one `git checkout`/`revert`, no token-burning hunt for scattered debug lines.
- **Self-closing crash (no dialog): WER dumps + `frida_crash_guard` can miss it.** Bisect with `Iwd2DebugLog`, launch with the selected runtime adapter and inspect the fresh log; the last entry narrows the fault. Stop only this checkout's process before retrying; preserve save data when investigating `tempsave` failures.
- Native Windows: `python scripts/native.py frida <script.py>` runs a probe in the current desktop session. Linux/VM: use a session-1 payload (`vm.sh frida`).
- **`vm.sh frida` payload gotchas (scheduled-task + fire-and-forget VBS — bite EVERY time):**
  - NO stdin: `sys.stdin.read()`=EOF → driver exits early. Keep alive: `while True: time.sleep(0.5)`.
  - stdout `>`-redirect drops writes once VBS parent exits. Write to a DEDICATED file w/ `f.flush()`+`os.fsync(fd)`, read THAT (not `vm_s1_out.txt`).
  - Re-attach: stuck driver BLOCKS `frida.attach` (silent no-fire / locked logfile). `taskkill` UNRELIABLE; identify and stop only the stale probe driver PID, then confirm it detached before re-ship.
  - Silent no-fire? base `0x400000` no-ASLR (`ptr(0xADDR)` absolute ok) — but dump 8 bytes/hook, diff vs `sym.py bytes ADDR` before blaming the path.
- `ptr(0xADDR)` absolute. `__thiscall`: `this` = `this.context.ecx`, stack args = `args[0..]`. Hook fn ENTRIES only.
- Runtime wins when `__thiscall` args disagree with decompiler; decompiler wins for counts (high-word garbage in int32 reads of 16-bit values).
- Read PE constants with `sym.py`, not the memory endpoint.
- Input-driving order: Frida-RPC into the engine fn > keyboard > `PostMessage` > polite physical hijack (unattended only; IWD2 polls mouse from the engine tick). Hover first and prove `CGameSprite::IsOver`/`OnActionButtonDown picked=...`; screenshots alone mislead. Dialog replies: `responseMarker` + real/post click.

## Game assets (`data/near_infinity_export/`)

NearInfinity export. Use for game content (action IDs, spell IDs, scripts, UI layouts).
Dirs: `2DA/` rule tables · `ARE/` areas · `BAM_DECOMP/` decompressed BAMv1 · `BCS/`+`BS/` scripts (.BAF) · `CHU/` UI panels · `CRE/` creatures · `DLG/` dialogs · `EFF/` effects · `IDS/` symbol→id maps · `ITM/` `SPL/` `STO/` `WED/` `WMP/`.

| Question | File |
|---|---|
| Action/trigger id N? | `IDS/ACTION.IDS`, `IDS/TRIGGER.IDS` |
| Stat/state name? | `IDS/STATS.IDS`, `IDS/SPLSTATE.IDS` |
| Spell SPxxx? / button slot? / qslot layout? | `SPL/SPxxx.SPL` (dump: `scripts/spell_info.py RESREF`) / `CHU/<panel>.CHU` / `2DA/QSLOTS.2DA` |
| Area edges? / script usage? | `WMP/WORLDMAP.WMP` / grep `BCS/*.BAF` + `BS/*.BAF` |

### Asset names / StringRefs

Resolve the canonical TLK name before naming an asset in comments or commit messages (never infer from resref):
`python scripts/reagent_asset_names.py SPWI304 AR1000 60SPELLS` (`--strref 6618`, `--json`). Output comment-ready: `Fireball (SPWI304.SPL, strref 6618)`. Supports SPL, ITM, CRE, STO, ARE.

## External refs

| Path | Use |
|-----|-----|
| `data/pdb/bg2_pdb_extracted.zip` / optional `Baldur.pdb` | BG2EE 2.5 names (offsets differ). `bg2_find.py` reads the bundled dumps when raw PDB/LLVM is absent |
| `refs/gemrb/` | GemRB source → CRG graph (`repo_root=".../refs/gemrb"`) |
| `refs/NearInfinity/` | File formats (.CRE/.ARE/.ITM) |
| `refs/iesdp/` | Effects, opcodes, STATS.IDS |
| `ghidra-projects/IWD2` (Windows), configured project path (Linux) | Ghidra project. CLOSE the GUI before `ghidra-bridge export`. |

## Temp files

`tmp_*.txt`, `tmp_*.json`, `chunk_*.sql` = RE session noise. Not tracked. Delete freely.
`.venv-reagent/`, `.ghidra-exports/` = toolchain + export cache. Gitignored.

## Code changes

- Recover → context bundle first (above). Verify `// 0xADDR` against Ghidra before touching.
- Prefer named constants over magic numbers when defined in file.
- `python scripts/vtable_audit.py ClassName` — missing virtual overrides (1-vtable anchor → conflation-blind).
- `python scripts/ctor_vtable_check.py [ClassName]` — class CONFLATION: ctors install diff vtables = 2 binary classes merged → wrong virtual recovered (the Fireball green bug). Run on classes w/ >1 ctor.
- After a recover: `re-agent parity --address 0xADDR` should be GREEN/YELLOW.
- Member-heavy fn (cell/render/anim): `python scripts/parity_offsets.py 0xADDR` — "right callee, wrong member" (diffs binary's per-thiscall `this`-offset→member vs source; parity call-counts blind to it; found the 3-yr corpse-tint bug). `scripts/lint_extend_cells.py` = source-only twin for extend/`*Base` copy-paste.
- `scripts/arg_provenance.py 0xADDR|--sweep` — binary operand-ORDER audit at `operator+` sites (the swap class parity is order-blind to; found FindFileInDirectoryList dir/file).
- `scripts/lint_twin_symmetry.py [--aggressive]` — source-only antonym-pair operand swap (LHS `<side>`-named, RHS pulls the opposite side whose same-side twin exists; left/right, top/bottom, src/dst, ... default — x/y/z, r/g/b, min/max FP-prone → `--aggressive`). Parity- AND arg_provenance-blind (locals in `-`/`/`, not `operator+`). Found the FillConvexPoly `nRightAdjUp`→`nLeftAdjUp` swap (0x7C0F40); catches only ASYMMETRIC swaps (both-sides-wrong stays asm/runtime).
- **Binary-mirror class (`/* 0xNNN */` offset comments) MUST be `#pragma pack(2)`** — IE packs to 2; without it a sub-4-aligned `LONG`/`WORD` tail silently 4-aligns. Parity stays GREEN (layout-blind) — only a runtime/visual bug shows (the Cloudkill ring over-density). `python scripts/struct_layout_audit.py ClassName [--header f.h --source f.cpp]` flags ACTIONABLE pack(2) drift vs benign STL/`CVidCell`/out-of-order breaks; offsetof can't (MSVC C2618 on polymorphic classes).
- **Static parity ≠ runtime proof — arc not done until OUR build runs the path.** Windows: `python scripts/native.py smoke 3 90` (or `--new-game --party 0 --hold 90`); Linux: `scripts/vm.sh smoke 3 90`. These load our build and observe it with the crash oracle. Native smoke arms the oracle before resuming and measures the hold interval after world activation. Logs: `tmp_smoke_guard.log`, plus `tmp_native_status.txt`/`tmp_native_load.txt` on Windows or `vm_s1_out.txt` on VM. Original-only Frida never exercises our exe. Behavioral claims need a matching original/recovered scenario, not just a clean crash guard.
- **Drive the UI yourself before asking the user.** `src/AutoUI.{h,cpp}` (test scaffolding, patterned on `AutoLoad.cpp`, inert unless `IWD2_RE_UI_SCRIPT` is set) replays a script from the tail of `CChitin::AsynchronousUpdate`. IWD2 handles no `WM_LBUTTONDOWN` — it polls `GetAsyncKeyState` and calls `pActiveEngine->OnLButtonDown/Up` with a screen point, so a synthetic click is those same two virtual calls. One primitive covers menus, action bar, portraits and ground; a control is addressed as `(panel, control)` and resolved to its centre, never as a pixel pair.
  - Run: `python scripts/native.py smoke 3 60 --ui scripts/scenarios/save-menu.txt` on Windows; `scripts/vm.sh smoke ...` on Linux. Composes with `--hit`: "the scenario clicked X" + "symbol Y fired" is what proves a UI action reached code. Output → `tmp_ui_result.jsonl` (one JSON per step). Exit 2 adds `UI-FAIL` / `UI-NO-RESULT` / `UI-INCOMPLETE`.
  - Ops: `wait <screen> [ticks]` · **`waitgo`** · `dump` · `screen` · `click <panel> <ctrl>` · `clickxy <x> <y>` · `key <vk>` · `goto <screen>` · `expect screen|controls|control` · `sleep <ticks>`.
  - **Always `waitgo` before the first action**, then wait for the intended screen. This synchronizes with the observer on both routes; native smoke arms before process resume, while the VM guard attaches later.
  - `dump` is the diagnostic: it answers "does this screen actually have controls, and are they active" without reading pixels. That is how the empty save menu was traced to `CScreenSave::RefreshGameSlots`.
- **A bare `RESULT: CLEAN` does NOT prove the recovered code ran.** Add `--hit Class::Method` to the selected smoke command: it counts entries via PDB symbols and exits **2** on `NOT-EXERCISED` or `NOT-INSTRUMENTED`. A comma-separated native/VM hit list requires every symbol to reach `--hit-min`. `--expect REGEX` checks the fresh debug log for deliberate `Iwd2DebugLog` instrumentation. `arc verify` supplies one resolved symbol; `arc batch` supplies all resolved symbols automatically. Exit 2, including harness errors, is unproven and must never become a plain PASS.

## Reproducible native profiles

```powershell
& $py scripts/arc.py batch 0x75E330 0x75DFD0 --profile scripts/profiles/combat-ammo.json
& $py scripts/arc.py explain --run RUN_ID
& $py scripts/arc.py explain smoke --run RUN_ID
```

The example reuses the existing ammunition diagnostic and its fixtures.
Profiles are schema-1 JSON with `save` (exact MPSave directory name), `ui`
(repository-relative scenario), `hold` (positive observation seconds),
`fixtures` (currently only `combat-payload`), optional `preserve` (additional
game-relative paths), and descriptive `preconditions`. Preconditions must be
checked by the scenario/probe; their prose alone is not an assertion. In
particular, set an exhausted item's charges explicitly after construction and
verify ability/stat requirements before testing an effect's recipient.

A profile owns the runtime settings and requires all target symbols. Conflicting
`--hit`, `--expect`, runtime overrides and `--no-build/--no-smoke/--no-parity`
are rejected. `--static` remains available for investigation and performs no
fixture setup or runtime work. Full profiles require PE, exports, Ghidra and
the native runtime capability; they cannot silently skip those gates. Existing
profile-free commands and the Linux/VM route remain available.

Runtime setup checks fixture collisions, validates path containment, acquires
an exclusive game-directory lock and stops only this checkout's executable.
It backs up `Icewind2.ini`, `temp`, `tempsave`, the selected MPSave directory,
the debug log/enable flag and declared additional paths. It enables logging for
the diagnostic and installs known fixtures. An explicit AutoUI FAIL now stops
the native smoke immediately, including before loading; incomplete captures
still wait for completion instead of being treated as failures.
Cleanup runs after success, error or Ctrl-C, stops a surviving checkout process,
removes owned fixtures, restores preserved paths and verifies their hashes.
Different fixture contents are preserved and reported as cleanup errors.
No original executable is killed by this profile wrapper.

Evidence lives in `.arc/RUN_ID/`: resolved profile, `run.json`, step outputs,
`continuation.md` and `profile-runtime/` with unique UI results, fresh runtime
logs, backups and a transaction journal. A hard OS/process termination cannot
run Python cleanup. In that case, or on restoration failure, the lock and
journal are deliberately retained: stop the checkout, inspect/restore the
recorded paths, and remove the lock only after resolving the journal. Never
blindly delete the transaction directory or an unfamiliar lock.

Each target has separate `evidence.static`, `evidence.runtime` and
`evidence.differential` entries. A bare CLEAN, a missing symbol counter, skipped
runtime or a static-only result cannot become an observed target. Static
checks passing is not semantic equivalence. The differential field remains
`unproven` until there is a separately recorded matched capture; a profile does
not fabricate an original-game oracle from reconstruction assertions.
An offset check reporting `not analysable` is skipped and leaves static evidence
`unproven`. Other unhandled nonzero tool exits and empty/incomplete parity
reports are errors. The console names targets with missing static checks even
when the overall operational verdict is PASS with warnings. Consult the
separate evidence fields before counting validated functions.
