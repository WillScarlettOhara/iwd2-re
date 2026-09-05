#!/usr/bin/env bash
# vm.sh - one-command facade for the Win11 VM (kills the ssh/scp/iconv/taskkill ceremony).
#
#   scripts/vm.sh build [--run]        sync+build via remote_build.sh, print ONLY errors/warnings
#                                      (full log -> tmp_vm_build.log). --run: kill ours + launch s1.
#   scripts/vm.sh run [slot]           kill ours, launch game in session 1 (default save slot 3)
#   scripts/vm.sh smoke [slot] [secs] [--hit SYM [--hit-min N]] [--expect RE [--expect-min N]]
#                                      arc gate: run OUR build + load save (default slot 3) + arm the
#                                      crash guard; interactive HOLDS until ENTER (= RESULT: CLEAN) or a
#                                      fault. Non-TTY (piped/backgrounded) auto-ends CLEAN after [secs]
#                                      of no fault (default 90s) so it never hangs. Symbolized bt on crash.
#                                      Exit 0 CLEAN / 1 CRASH / 2 the path never ran (see below).
#                                      --hit SYM   counts entries to SYM via the guard's PDB symbols
#                                                  (no source change); 0 hits -> RESULT: NOT-EXERCISED,
#                                                  unresolvable -> RESULT: NOT-INSTRUMENTED. Both exit 2.
#                                      --expect RE greps the debug log instead, for the flow where you
#                                                  deliberately added Iwd2DebugLog in an uncommitted tree.
#                                      Without either, a CLEAN only means "no fault while idle".
#   scripts/vm.sh log <regex> [-n 50] [-f <vm-path>]
#                                      Select-String server-side (UTF-16 safe, no iconv), last N
#   scripts/vm.sh tail [N] [-f <vm-path>]   last N raw lines of the debug log
#   scripts/vm.sh clearlog [-f <vm-path>]   truncate the debug log (auto-run by run/smoke)
#   scripts/vm.sh cleantemp            reap leaked frida-<hash> temp dirs in %LOCALAPPDATA%\Temp
#                                      (auto-run by frida/smoke; ~52 MB each, skips in-use ones)
#   scripts/vm.sh status               cat vm_s1_out.txt (launch/smoke status)
#   scripts/vm.sh ps                   iwd2/frida processes on the VM
#   scripts/vm.sh kill [ours|orig|all] taskkill: ours=iwd2-re.exe (default), orig=IWD2.exe.
#                                      NEVER called implicitly by `build`; `run` kills ours only
#                                      (relaunch implies replacing the instance; Frida sessions on
#                                      the original survive builds).
#   scripts/vm.sh pull <vm-path> [dst] scp from VM (default dst .)
#   scripts/vm.sh push <src> <vm-path> scp to VM
#   scripts/vm.sh frida <script.py> [args...]
#                                      scp script + run it as the session-1 payload, ready-to-log
#   scripts/vm.sh trace --hooks <table.json> [--load-slot N] [--hit NAME [--hit-min N]]
#                       [--settle-ticks N] [--post-load S] [--timeout S] [--out NAME]
#                                      smoke's counterpart for the ORIGINAL IWD2.exe: spawn it under
#                                      Frida, drive it into a loaded save by CALLING the engine (not
#                                      keystrokes), run the hook table, WAIT for the verdict. Use it
#                                      when the static read cannot answer a runtime question.
#                                      Exit 0 CLEAN / 1 CRASH / 2 NOT-LOADED|NOT-EXERCISED|NO-VERDICT.
#                                      Trace -> tmp_orig_trace.jsonl; crash frames symbolized via
#                                      sym.py (the original has no PDB). Table schema: frida_hooks.py.
#                                      Gets into a save by itself: skips the movies, dismisses the
#                                      network popup, calibrates the cursor, clicks Load Game then
#                                      Done on arbitration. --load-slot -1 stops at the menu.
set -euo pipefail

VM="win11vm"
VM_REPO='C:/iwd2-re'
DEFAULT_LOG='C:\GOG Games\Icewind Dale 2\iwd2-re-debug.log'
HERE="$(cd "$(dirname "$0")" && pwd)"
REPO="$(dirname "$HERE")"

ps_quote() { printf "%s" "${1//\'/\'\'}"; }   # double single-quotes for PS single-quoted strings

# Feed the PS script over stdin: the VM's default ssh shell is PowerShell itself,
# so any inline quoting gets a second round of parsing (and eats $_).
vm_ps() { printf '%s\n' "$1" | ssh "$VM" "powershell -NoProfile -Command -"; }

# Truncate the debug log so the next capture starts clean. Safe to call while the
# game is down (run/smoke kill our exe first); on a locked file it no-ops.
clear_log() {
  local f="${1:-$DEFAULT_LOG}"
  vm_ps "if (Test-Path '$(ps_quote "$f")') { Clear-Content -Path '$(ps_quote "$f")' -ErrorAction SilentlyContinue }" >/dev/null 2>&1 || true
}

# Frida leaks one ~52 MB frida-<hash> dir (the embedded frida-agent) per attach/
# spawn under %LOCALAPPDATA%\Temp and never reaps them -- 35 stale dirs (1.8 GB)
# had piled up. Rename-probe each: a LIVE session holds the agent DLL so renaming
# its dir fails with a sharing violation and we skip it untouched. We only delete
# dirs that rename cleanly == no process has them open, so there is no risk of
# half-deleting an in-use session. Best-effort: never fails the caller.
clean_frida_temp() {
  vm_ps '
    $t = Join-Path $env:LOCALAPPDATA "Temp"
    $freed = 0; $n = 0; $skip = 0
    Get-ChildItem -Path $t -Directory -Filter "frida-*" -ErrorAction SilentlyContinue | ForEach-Object {
      $leaf = ".reap-" + [guid]::NewGuid().ToString("N")
      try {
        Rename-Item -LiteralPath $_.FullName -NewName $leaf -ErrorAction Stop
        $moved = Join-Path $t $leaf
        $sz = (Get-ChildItem -LiteralPath $moved -Recurse -File -ErrorAction SilentlyContinue | Measure-Object -Property Length -Sum).Sum
        Remove-Item -LiteralPath $moved -Recurse -Force -ErrorAction SilentlyContinue
        $freed += $sz; $n++
      } catch { $skip++ }
    }
    Write-Output ("frida temp: reaped {0} dir(s), {1:N1} MB freed; {2} in-use skipped" -f $n, ($freed/1MB), $skip)
  ' 2>/dev/null || true
}

cmd="${1:-}"; shift || true
case "$cmd" in
  build)
    log="$REPO/tmp_vm_build.log"
    if "$HERE/remote_build.sh" "$@" >"$log" 2>&1; then ok=1; else ok=0; fi
    # surface: orchestrator phases, compiler/linker diagnostics, terminal status
    grep -E '^==>|^   (status|WARN)|(: |LINK : )(fatal )?(error|warning) [A-Z]*[0-9]+|LNK[0-9]+|^done\.' "$log" \
      | sed 's/^[[:space:]]*//' | awk '!seen[$0]++' | head -60 || true
    n_err=$(grep -cE ': (fatal )?error ' "$log" || true)
    if [ "$ok" = 1 ] && [ "${n_err:-0}" = "0" ]; then
      echo "BUILD OK  (full log: tmp_vm_build.log)"
    else
      echo "BUILD FAILED ($n_err errors)  full log: tmp_vm_build.log"
      grep -E ': (fatal )?error ' "$log" | grep -q 'LNK1104.*iwd2-re' \
        && echo "hint: iwd2-re.exe is running and locks the exe -> scripts/vm.sh kill ours"
      exit 1
    fi
    ;;
  run)
    slot="${1:-}"
    if [ -n "$slot" ]; then
      sed -i "s/--slot [^ ]*/--slot $slot/" "$HERE/vm_s1_payload.cmd"
      scp -q "$HERE/vm_s1_payload.cmd" "$VM:$VM_REPO/scripts/"
    fi
    ssh "$VM" 'cmd /c "taskkill /im iwd2-re.exe /f >nul 2>&1 & exit 0"' >/dev/null || true
    clear_log   # fresh capture each launch (exe is down -> log unlocked)
    ssh "$VM" "cmd /c $VM_REPO/scripts/vm_s1.cmd"
    for i in $(seq 1 30); do
      out=$(ssh "$VM" 'cmd /c "type C:\iwd2-re\vm_s1_out.txt 2>nul"' 2>/dev/null || true)
      case "$out" in *loaded:*|*timeout*|*failed:*) echo "status: $out"; exit 0 ;; esac
      sleep 5
    done
    echo "WARN: no terminal status after 150s (last: '${out:-<empty>}')"
    ;;
  smoke)
    # Arc gate: exercise the recovered path on OUR build with the crash oracle armed.
    # Closes the gap that shipped the Fireball crashes (validated on the original only,
    # never run on our exe). Interactive: holds until you confirm the cast or it faults.
    # positionals first ([slot] [secs]), then flags in any order
    slot=""; hold=""; hit=""; hit_min=1; expect=""; expect_min=1; ui=""; uiverdict=""; save=""
    while [ $# -gt 0 ]; do
      case "$1" in
        --save)       save="$2"; shift 2 ;;
        --hit)        hit="$2"; shift 2 ;;
        --hit-min)    hit_min="$2"; shift 2 ;;
        --expect)     expect="$2"; shift 2 ;;
        --expect-min) expect_min="$2"; shift 2 ;;
        --ui)         ui="$2"; shift 2 ;;
        --*) echo "smoke: unknown flag $1" >&2; exit 2 ;;
        # With --save there is no slot to give, so the positionals are [secs]
        # only -- otherwise `smoke --save X 300` silently reads 300 as the slot
        # and falls back to the default hold.
        *) if [ -n "$save" ]; then
             if [ -z "$hold" ]; then hold="$1"; fi
           elif [ -z "$slot" ]; then slot="$1"
           elif [ -z "$hold" ]; then hold="$1"
           fi
           shift ;;
      esac
    done
    slot="${slot:-3}"
    # Optional auto-hold (seconds): end with RESULT: CLEAN after N seconds of no
    # fault, for a non-interactive driver. Defaults to 90s whenever stdin is not a
    # TTY, so a backgrounded/piped smoke can never hang the way the old ENTER-only
    # loop did ([ -t 0 ] rejected piped ENTER -> spun on `while kill -0 gpid`).
    if [ ! -t 0 ] && [ -z "$hold" ]; then hold=90; fi
    # Regenerate the payload rather than sed-patching it: --ui adds arguments,
    # not just a different slot number.
    uiargs=""
    if [ -n "$ui" ]; then
      [ -f "$ui" ] || { echo "smoke: no such ui script: $ui" >&2; exit 2; }
      scp -q "$ui" "$VM:$VM_REPO/tmp_ui_script.txt"
      ssh "$VM" 'cmd /c "del /q C:\iwd2-re\tmp_ui_result.jsonl 2>nul & exit 0"' >/dev/null 2>&1 || true
      ssh "$VM" 'cmd /c "del /q C:\iwd2-re\tmp_ui_go.txt 2>nul & exit 0"' >/dev/null 2>&1 || true
      # Drop our copy too: the pull below tolerates failure, so a stale local
      # file would be read as this run's result and report the PREVIOUS verdict.
      rm -f "$REPO/tmp_ui_result.jsonl"
      uiargs=" --ui-script C:\\iwd2-re\\tmp_ui_script.txt --ui-result C:\\iwd2-re\\tmp_ui_result.jsonl --ui-go C:\\iwd2-re\\tmp_ui_go.txt"
    fi
    # --save NAME wins over the positional slot: an MPSave directory name is stable,
    # a visible load-screen index shifts every time a save is added.
    loadargs="--slot $slot"
    if [ -n "$save" ]; then loadargs="--save-name \"$save\""; fi
    printf '@echo off\r\nREM Generated by vm.sh smoke -- do not edit.\r\ncd /d C:\\iwd2-re\r\npython scripts\\auto_start_game.py %s%s > C:\\iwd2-re\\vm_s1_out.txt 2>&1\r\n' \
      "$loadargs" "$uiargs" > "$HERE/vm_s1_payload.cmd"
    # Ship the launcher too: remote_build.sh syncs it, but only on a build, so a
    # smoke right after editing it would silently run the VM's stale copy.
    scp -q "$HERE/vm_s1_payload.cmd" "$HERE/auto_start_game.py" "$HERE/runtime_paths.py" "$VM:$VM_REPO/scripts/"
    ssh "$VM" 'cmd /c "taskkill /im iwd2-re.exe /f >nul 2>&1 & exit 0"' >/dev/null || true
    clear_log   # fresh capture each launch (exe is down -> log unlocked)
    ssh "$VM" "cmd /c $VM_REPO/scripts/vm_s1.cmd"
    if [ -n "$save" ]; then what="save $save"; else what="slot $slot"; fi
    echo "==> launching our build ($what), waiting for load..."
    out=""
    for i in $(seq 1 30); do
      out=$(ssh "$VM" 'cmd /c "type C:\iwd2-re\vm_s1_out.txt 2>nul"' 2>/dev/null || true)
      case "$out" in
        *loaded:*) break ;;
        *timeout*|*failed:*) echo "launch FAILED: $out"; exit 1 ;;
      esac
      sleep 5
    done
    case "$out" in *loaded:*) : ;; *) echo "WARN: no load status (last: '${out:-<empty>}'); arming guard anyway" ;; esac
    pid=$(ssh "$VM" 'powershell -NoProfile -Command "(Get-Process iwd2-re -ErrorAction SilentlyContinue | Select-Object -First 1 -ExpandProperty Id)"' 2>/dev/null | tr -d "\r")
    [ -z "$pid" ] && { echo "ERROR: iwd2-re.exe not running; nothing to guard"; exit 1; }
    scp -q "$HERE/frida_crash_guard.py" "$VM:$VM_REPO/scripts/"
    glog="$REPO/tmp_smoke_guard.log"; : > "$glog"
    # attach-by-pid as a host-side bg ssh child; its stdout streams back reliably
    # (unlike the vm.sh-frida VBS payload). </dev/null so it never steals our ENTER.
    countarg=""
    [ -n "$hit" ] && countarg=" --count '$(ps_quote "$hit")'"
    ssh "$VM" "python $VM_REPO/scripts/frida_crash_guard.py $pid$countarg" </dev/null >"$glog" 2>&1 &
    gpid=$!
    for i in $(seq 1 20); do grep -qE "ARMED|ATTACH_FAILED" "$glog" 2>/dev/null && break; sleep 1; done
    if grep -q ATTACH_FAILED "$glog" 2>/dev/null; then
      echo "guard attach FAILED:"; cat "$glog"; kill "$gpid" 2>/dev/null || true; exit 1
    fi
    # Release the scenario only now: before this point --hit would miss every
    # click, because the guard was not attached yet.
    if [ -n "$ui" ]; then
      ssh "$VM" 'cmd /c "echo go > C:\iwd2-re\tmp_ui_go.txt & exit 0"' >/dev/null 2>&1 || true
    fi
    echo
    echo "==> CRASH GUARD ARMED on pid $pid (slot $slot loaded)."
    [ -n "$hit" ]    && echo "    Watching entries to '$hit' (need >= $hit_min)."
    [ -n "$expect" ] && echo "    Watching the debug log for /$expect/ (need >= $expect_min)."
    echo "    Drive the recovered path in the VM window now (cast the spell / trigger the code)."
    if [ -n "$hold" ]; then
      echo "    Auto-hold: ${hold}s with no fault  ->  RESULT: CLEAN (non-interactive)."
    else
      echo "    Press ENTER here when done with NO crash  ->  RESULT: CLEAN."
    fi
    echo "    If it faults, the symbolized backtrace prints here automatically."
    echo
    verdict=""
    end=0
    [ -n "$hold" ] && end=$(( $(date +%s) + hold ))
    while kill -0 "$gpid" 2>/dev/null; do
      if grep -q "EXCEPTION" "$glog" 2>/dev/null; then verdict="CRASH"; break; fi
      if [ -n "$hold" ]; then
        [ "$(date +%s)" -ge "$end" ] && { verdict="CLEAN"; break; }
        sleep 3
      elif read -t 3 -r _; then
        sleep 1
        grep -q "EXCEPTION" "$glog" 2>/dev/null && verdict="CRASH" || verdict="CLEAN"
        break
      fi
    done
    [ -z "$verdict" ] && { grep -q "EXCEPTION" "$glog" 2>/dev/null && verdict="CRASH" || verdict="CLEAN"; }
    # teardown: stop the (possibly still-attached) remote guard so it can't block the next smoke
    kill "$gpid" 2>/dev/null || true
    ssh "$VM" 'powershell -NoProfile -Command "Stop-Process -Name python -Force -ErrorAction SilentlyContinue"' >/dev/null 2>&1 || true
    clean_frida_temp >/dev/null 2>&1 || true   # reap the crash-guard's frida temp dir (python now dead -> unlocked)
    # leave no idle game behind: stop OUR exe now (a later run/smoke would only kill
    # it at start anyway, so it would otherwise sit burning CPU between sessions).
    # Per kill policy smoke owns iwd2-re.exe; the original IWD2.exe is never touched.
    ssh "$VM" 'cmd /c "taskkill /im iwd2-re.exe /f >nul 2>&1 & exit 0"' >/dev/null 2>&1 || true
    echo
    if [ "$verdict" = "CRASH" ]; then
      echo "===================== RESULT: CRASH ====================="
      sed -n '/EXCEPTION/,$p' "$glog"
      echo "========================================================"
      echo "(full guard log: tmp_smoke_guard.log)"
      exit 1
    fi

    # A UI scenario is the strongest evidence available: it says which screens
    # were reached and which assertions held. Pull it before anything else.
    if [ -n "$ui" ]; then
      scp -q "$VM:$VM_REPO/tmp_ui_result.jsonl" "$REPO/tmp_ui_result.jsonl" 2>/dev/null || true
      if [ ! -s "$REPO/tmp_ui_result.jsonl" ]; then
        echo "=================== RESULT: UI-NO-RESULT ==================="
        echo "AutoUI wrote nothing. The build did not crash, but the scenario"
        echo "never ran -- check that this exe has AutoUI compiled in."
        exit 2
      fi
      verdictline=$(grep '"op":"verdict"' "$REPO/tmp_ui_result.jsonl" | tail -1)
      if [ -z "$verdictline" ]; then
        echo "=================== RESULT: UI-INCOMPLETE ==================="
        echo "scenario started but never reached its verdict (last lines):"
        tail -3 "$REPO/tmp_ui_result.jsonl"
        echo "(full log: tmp_ui_result.jsonl)"
        exit 2
      fi
      case "$verdictline" in
        *'"verdict":"PASS"'*)
          # Do NOT exit here: --ui composes with --hit, and that pairing is the
          # whole point -- "the scenario clicked X" plus "symbol Y then fired"
          # is what proves a UI action actually reached the code.
          uiverdict="PASS" ;;
        *)
          echo "===================== RESULT: UI-FAIL ====================="
          grep '"status":"fail"' "$REPO/tmp_ui_result.jsonl" | head -5
          echo "$verdictline"
          echo "(full log: tmp_ui_result.jsonl)"
          exit 2 ;;
      esac
    fi

    # No fault is only half the answer: it does not distinguish "the recovered
    # code is correct" from "the recovered code never ran". Exit 2 is that third
    # state -- ran clean, proved nothing -- and callers must not read it as a pass.
    if [ -n "$hit" ]; then
      IFS=',' read -r -a hit_symbols <<< "$hit"
      missing=""; under=""; hit_summary=""
      for symbol in "${hit_symbols[@]}"; do
        symbol="${symbol#${symbol%%[![:space:]]*}}"
        symbol="${symbol%${symbol##*[![:space:]]}}"
        [ -z "$symbol" ] && continue
        if grep -Fxq "NOT_INSTRUMENTED $symbol" "$glog" 2>/dev/null; then
          missing="${missing:+$missing, }$symbol"
          continue
        fi
        n=$(grep -F "HITS $symbol " "$glog" 2>/dev/null | tail -1 | awk '{print $NF}')
        n="${n:-0}"
        hit_summary="${hit_summary:+$hit_summary; }$symbol hit x$n"
        if [ "$n" -lt "$hit_min" ]; then
          under="${under:+$under, }$symbol=$n"
        fi
      done
      if [ -n "$missing" ]; then
        echo "================= RESULT: NOT-INSTRUMENTED ================="
        echo "symbol(s) '$missing' resolved to no address in our build."
        echo "inlined, folded by /OPT:ICF, or the wrong name -- this smoke proves nothing."
        exit 2
      fi
      if [ -n "$under" ]; then
        echo "=================== RESULT: NOT-EXERCISED =================="
        echo "symbol counts below $hit_min in ${hold:-the session}s: $under"
        echo "the build did not crash, but every requested path must reach the gate."
        [ -n "$hit_summary" ] && echo "observed: $hit_summary"
        exit 2
      fi
      echo "RESULT: CLEAN  ($hit_summary; guard log: tmp_smoke_guard.log)"
      exit 0
    fi

    if [ -n "$expect" ]; then
      # clear_log ran before launch, so this count is this run's, not history.
      hits=$(vm_ps "@(Select-String -Path '$(ps_quote "$DEFAULT_LOG")' -Pattern '$(ps_quote "$expect")').Count" 2>/dev/null | tr -d '\r' | tail -1)
      hits="${hits:-0}"
      if [ "$hits" -lt "$expect_min" ]; then
        echo "=================== RESULT: NOT-EXERCISED =================="
        echo "no /$expect/ in the debug log after ${hold:-the session}s (got $hits, need >= $expect_min)."
        echo "the build did not crash -- but the recovered path never ran, so this proves nothing."
        exit 2
      fi
      echo "RESULT: CLEAN  (marker /$expect/ x$hits; guard log: tmp_smoke_guard.log)"
      exit 0
    fi

    if [ -n "$uiverdict" ]; then
      echo "RESULT: CLEAN  (ui scenario PASS; log: tmp_ui_result.jsonl)"
      exit 0
    fi
    echo "RESULT: CLEAN  (no fault only -- nothing proved the path ran; pass --hit or --ui to gate on that)"
    ;;
  log)
    pat="${1:?usage: vm.sh log <regex> [-n N] [-f vm-path]}"; shift
    n=50; f="$DEFAULT_LOG"
    while [ $# -gt 0 ]; do case "$1" in -n) n="$2"; shift 2;; -f) f="$2"; shift 2;; *) shift;; esac; done
    vm_ps "Select-String -Path '$(ps_quote "$f")' -Pattern '$(ps_quote "$pat")' | Select-Object -Last $n | ForEach-Object { \$_.LineNumber.ToString() + ':' + \$_.Line }"
    ;;
  tail)
    n="${1:-30}"; f="$DEFAULT_LOG"
    [ "${2:-}" = "-f" ] && f="$3"
    vm_ps "Get-Content -Tail $n -Path '$(ps_quote "$f")'"
    ;;
  clearlog)
    f="$DEFAULT_LOG"; [ "${1:-}" = "-f" ] && f="$2"
    clear_log "$f"; echo "cleared: $f"
    ;;
  cleantemp)
    clean_frida_temp
    ;;
  status)
    ssh "$VM" 'cmd /c "type C:\iwd2-re\vm_s1_out.txt 2>nul"' || echo "(no vm_s1_out.txt)"
    ;;
  ps)
    ssh "$VM" 'cmd /c "tasklist | findstr /i \"iwd2 IWD2 frida python\" & exit 0"'
    ;;
  kill)
    tgt="${1:-ours}"
    case "$tgt" in
      ours) imgs="iwd2-re.exe" ;;
      orig) imgs="IWD2.exe" ;;
      all)  imgs="iwd2-re.exe IWD2.exe" ;;
      *) echo "kill ours|orig|all"; exit 2 ;;
    esac
    for im in $imgs; do
      ssh "$VM" "cmd /c \"taskkill /im $im /f & exit 0\"" || true
    done
    ;;
  pull)
    src="${1:?vm.sh pull <vm-path> [dst]}"; dst="${2:-.}"
    scp -q "$VM:$src" "$dst" && echo "pulled -> $dst"
    ;;
  push)
    src="${1:?vm.sh push <src> <vm-path>}"; dst="${2:?vm.sh push <src> <vm-path>}"
    scp -q "$src" "$VM:$dst" && echo "pushed -> $dst"
    ;;
  trace)
    # The original's counterpart of `smoke`: spawn IWD2.exe under Frida in session 1,
    # let frida_orig.py drive it into a loaded save through engine calls, run the hook
    # table, and come back with a verdict. Unlike `frida` (fire-and-forget) this waits.
    hooks=""; slot=3; settle=10; postload=20; timeout=180; hit=""; hit_min=1
    outname="tmp_orig_trace.jsonl"
    while [ $# -gt 0 ]; do
      case "$1" in
        --hooks) hooks="$2"; shift 2 ;;
        --load-slot) slot="$2"; shift 2 ;;
        --settle-ticks) settle="$2"; shift 2 ;;
        --post-load) postload="$2"; shift 2 ;;
        --timeout) timeout="$2"; shift 2 ;;
        --hit) hit="$2"; shift 2 ;;
        --hit-min) hit_min="$2"; shift 2 ;;
        --out) outname="$2"; shift 2 ;;
        *) echo "vm.sh trace: unknown flag '$1'"; exit 2 ;;
      esac
    done
    [ -n "$hooks" ] || { echo "vm.sh trace --hooks <table.json> [--load-slot N] [--hit NAME] ..."; exit 2; }
    [ -f "$hooks" ] || { echo "no such hook table: $hooks"; exit 2; }
    hooksbase="$(basename "$hooks")"
    status="$outname.status"

    # Gotcha 4: a stuck driver from a previous run silently blocks frida.attach.
    # Clear it BEFORE shipping, not after, and take the old game down with it --
    # a leftover IWD2.exe would fight the new one for the session-1 desktop.
    ssh "$VM" 'powershell -NoProfile -Command "Stop-Process -Name python -Force -ErrorAction SilentlyContinue"' >/dev/null 2>&1 || true
    ssh "$VM" 'cmd /c "taskkill /im IWD2.exe /f >nul 2>&1 & exit 0"' >/dev/null 2>&1 || true
    clean_frida_temp >/dev/null 2>&1 || true
    scp -q "$HERE/frida_orig.py" "$HERE/frida_hooks.py" "$HERE/runtime_paths.py" "$hooks" "$VM:$VM_REPO/scripts/"
    ssh "$VM" "cmd /c \"del /q C:\\iwd2-re\\$outname C:\\iwd2-re\\$status >nul 2>&1 & exit 0\"" >/dev/null 2>&1 || true

    hitargs=""
    [ -n "$hit" ] && hitargs=" --hit \"$hit\" --hit-min $hit_min"
    printf '@echo off\r\nREM Generated by vm.sh trace -- do not edit.\r\ncd /d C:\\iwd2-re\r\npython scripts\\frida_orig.py --hooks scripts\\%s --out C:\\iwd2-re\\%s --load-slot %s --settle-ticks %s --post-load %s --timeout %s%s > C:\\iwd2-re\\vm_s1_out.txt 2>&1\r\n' \
      "$hooksbase" "$outname" "$slot" "$settle" "$postload" "$timeout" "$hitargs" > "$HERE/vm_s1_payload.cmd"
    scp -q "$HERE/vm_s1_payload.cmd" "$VM:$VM_REPO/scripts/"
    ssh "$VM" "cmd /c $VM_REPO/scripts/vm_s1.cmd"
    echo "==> tracing the ORIGINAL ($hooksbase, slot $slot), waiting for the verdict..."
    [ -n "$hit" ] && echo "    Watching entries to '$hit' (need >= $hit_min)."

    # Poll the DEDICATED status file, never vm_s1_out.txt: the VBS parent exits and
    # takes the stdout redirection with it (gotcha 2).
    st=""
    end=$(( $(date +%s) + timeout + 90 ))
    while [ "$(date +%s)" -lt "$end" ]; do
      st=$(ssh "$VM" "cmd /c \"type C:\\iwd2-re\\$status 2>nul\"" 2>/dev/null || true)
      case "$st" in *RESULT:*) break ;; esac
      sleep 5
    done
    ssh "$VM" 'powershell -NoProfile -Command "Stop-Process -Name python -Force -ErrorAction SilentlyContinue"' >/dev/null 2>&1 || true
    ssh "$VM" 'cmd /c "taskkill /im IWD2.exe /f >nul 2>&1 & exit 0"' >/dev/null 2>&1 || true
    clean_frida_temp >/dev/null 2>&1 || true
    scp -q "$VM:$VM_REPO/$outname" "$REPO/$outname" 2>/dev/null || true
    scp -q "$VM:$VM_REPO/$status" "$REPO/$status" 2>/dev/null || true

    case "$st" in
      *RESULT:*) : ;;
      *) echo "=================== RESULT: NO-VERDICT ==================="
         echo "the driver never wrote a verdict (last status: '${st:-<empty>}')."
         echo "vm_s1_out.txt is the place to look -- a python traceback lands there:"
         ssh "$VM" 'cmd /c "type C:\iwd2-re\vm_s1_out.txt 2>nul"' 2>/dev/null | tail -20 || true
         exit 2 ;;
    esac

    echo
    printf '%s\n' "$st" | grep -v '^  frame ' || true
    # The original has no PDB, so the guard's frames come back as bare addresses.
    # Ghidra knows them: resolve host-side rather than shipping symbols to the VM.
    if printf '%s\n' "$st" | grep -q '^  frame '; then
      echo "  --- symbolized backtrace ---"
      printf '%s\n' "$st" | sed -n 's/^  frame //p' | tr -d '\r' | while read -r fr; do
        "$REPO/.venv-reagent/bin/python" "$HERE/sym.py" addr2fn "$fr" 2>/dev/null || echo "  $fr  ?"
      done
    fi
    echo "(trace: $outname)"
    verdict=$(printf '%s\n' "$st" | sed -n 's/^RESULT: \([A-Z-]*\).*/\1/p' | tail -1)
    case "$verdict" in
      CLEAN) exit 0 ;;
      CRASH) exit 1 ;;
      *)     exit 2 ;;
    esac
    ;;
  frida)
    script="${1:?vm.sh frida <script.py> [args...]}"; shift
    base="$(basename "$script")"
    clean_frida_temp   # reap stale frida-<hash> temp dirs before this session adds another
    scp -q "$script" "$HERE/runtime_paths.py" "$VM:$VM_REPO/scripts/"
    printf '@echo off\r\ncd /d C:\\iwd2-re\r\npython scripts\\%s %s > C:\\iwd2-re\\vm_s1_out.txt 2>&1\r\n' \
      "$base" "$*" > /tmp/vm_s1_payload.cmd
    scp -q /tmp/vm_s1_payload.cmd "$VM:$VM_REPO/scripts/"
    ssh "$VM" "cmd /c $VM_REPO/scripts/vm_s1.cmd"
    echo "frida payload launched in session 1 (watch: vm.sh status / vm.sh tail)"
    ;;
  *)
    sed -n '2,26p' "$0"
    ;;
esac
