#!/usr/bin/env python3
"""arc -- run the iwd2-re verification sequence as one command with one verdict.

The recover loop's validation sequence used to live only as prose in CLAUDE.md:
parity -> parity_offsets -> struct_layout_audit -> ctor_vtable_check ->
vtable_audit -> lint_twin_symmetry -> arg_provenance -> vm.sh build -> vm.sh
smoke. Nine commands, several hundred lines of tool prose, nothing enforcing the
order and nothing failing when a step was skipped.

arc runs them, collects every result into one envelope, and prints a fixed-width
verdict of a dozen lines. Tool prose is never re-printed: it is captured under
.arc/<run>/<step>.{out,err} and surfaced by `arc explain <step>`.

  arc check [--staged|--all]      host-only source lints, ~2s     (the commit gate)
  arc verify 0xADDR               the full per-arc sequence
  arc sweep [--deep]              whole-codebase auditors
  arc status                      recovery metrics + freshness nags
  arc targets [...]               proxy to scripts/next_targets.py
  arc explain <step>              full captured output of a step from the last run
  arc hooks --install             (re-)install the pre-commit delegation

Exit codes:  0 = no failure   1 = at least one failure   2 = harness error.

A missing capability (no VM, no .ghidra-exports, Ghidra GUI open) yields a
`skip`, never a `fail` -- so arc is usable on a laptop with none of the
toolchain. Pass --require to turn specific skips into failures.

Stdlib only, imports nothing from the project: it must run under a bare python3
in a git hook. Children that need pefile/re_agent get .venv-reagent/bin/python.
"""

from __future__ import annotations

import argparse
import concurrent.futures
import json
import os
import re
import shutil
import subprocess
import sys
import threading
import time
from dataclasses import dataclass, field
from datetime import datetime
from pathlib import Path

from runtime_paths import WINDOWS, cppcheck_executable, python_executable, runtime_command, utf8_stdio, venv_tool

REPO = Path(__file__).resolve().parent.parent
SCRIPTS = REPO / "scripts"
ARCDIR = REPO / ".arc"
BASELINE = SCRIPTS / "arc-baseline.txt"
EXPORTS = REPO / ".ghidra-exports"

SCHEMA = 1

# Children that import pefile / re_agent need the project venv; arc itself does not.
PY = python_executable()

# Touching one of these means the change is visible on screen, and per project
# policy only the user's eye can sign that off -- arc must never claim otherwise.
VISUAL_PATTERNS = ("CVidCell", "CVidPalette", "CVidMode", "CProjectile",
                   "Render", "Anim", "Draw", "Blit")

STATUS_ORDER = {"fail": 0, "error": 1, "warn": 2, "skip": 3, "pass": 4}
TAG = {"pass": "ok  ", "warn": "WARN", "fail": "FAIL", "skip": "skip", "error": "ERR "}


# --------------------------------------------------------------------------
# result envelope (the schema every step produces, tool-agnostic)
# --------------------------------------------------------------------------

@dataclass
class Finding:
    severity: str = "fail"
    message: str = ""
    file: str | None = None
    line: int | None = None
    code: str | None = None
    symbol: str | None = None
    address: str | None = None
    baselined: bool = False

    def fingerprints(self) -> list[str]:
        """Keys a baseline entry may match on.

        Two keys, deliberately: file+tool+line is exact, file+tool+message
        survives a pure line shift when unrelated code above it moves. Without
        the second key every edit above a baselined finding would resurrect it.
        """
        out = []
        if self.file and self.line is not None:
            out.append(f"L|{self.file}|{self.line}")
        if self.file and self.message:
            out.append(f"M|{self.file}|{self.message.strip()[:120]}")
        return out

    def to_json(self) -> dict:
        d = {"severity": self.severity, "message": self.message}
        for k in ("file", "line", "code", "symbol", "address"):
            v = getattr(self, k)
            if v is not None:
                d[k] = v
        if self.baselined:
            d["baselined"] = True
        return d


@dataclass
class Report:
    tool: str
    status: str = "pass"
    exit_code: int | None = None
    summary: str = ""
    findings: list[Finding] = field(default_factory=list)
    notes: list[str] = field(default_factory=list)
    scope: dict = field(default_factory=dict)
    counts: dict = field(default_factory=dict)
    elapsed_s: float = 0.0
    label: str = ""
    advisory: bool = False

    def to_json(self) -> dict:
        return {
            "tool": self.tool,
            "schema": SCHEMA,
            "status": self.status,
            "advisory": self.advisory,
            "exit_code": self.exit_code,
            "summary": self.summary,
            "scope": self.scope,
            "counts": self.counts,
            "findings": [f.to_json() for f in self.findings],
            "notes": self.notes,
            "elapsed_s": round(self.elapsed_s, 2),
        }


# --------------------------------------------------------------------------
# baseline -- accepted pre-existing findings
# --------------------------------------------------------------------------

class Baseline:
    """Findings that are known-good and must not fail the gate.

    Without this, `arc check` is red on day one (lint_stuck_loop_index has a
    standing hit in CSpawn.cpp that is correct against the binary), and a red
    gate is a gate people switch off.
    """

    LINE_RE = re.compile(r"^(?P<file>[^:]+):(?P<line>\d+):\s*(?P<tool>\S+)")

    def __init__(self, path: Path = BASELINE):
        self.entries: list[tuple[str, str, int, str]] = []   # tool, file, line, why
        self.used: set[int] = set()
        self.path = path
        if not path.exists():
            return
        for raw in path.read_text(encoding="utf-8").splitlines():
            line = raw.strip()
            if not line or line.startswith("#"):
                continue
            body, _, why = line.partition("#")
            m = self.LINE_RE.match(body.strip())
            if m:
                self.entries.append(
                    (m["tool"], m["file"], int(m["line"]), why.strip()))

    def apply(self, tool: str, findings: list[Finding]) -> int:
        """Mark baselined findings; return how many were suppressed."""
        n = 0
        for f in findings:
            keys = set(f.fingerprints())
            for i, (btool, bfile, bline, _why) in enumerate(self.entries):
                if btool != tool or bfile != f.file:
                    continue
                if f"L|{bfile}|{bline}" in keys or (
                        f.line is not None and abs(f.line - bline) <= 200
                        and f"M|{bfile}|{f.message.strip()[:120]}" in keys):
                    f.baselined = True
                    self.used.add(i)
                    n += 1
                    break
        return n

    def stale(self, ran_tools: set[str],
              scope_files: set[str] | None = None) -> list[str]:
        """Entries that matched nothing, so the finding is gone or moved.

        Restricted to tools that actually ran AND, when the run was scoped to
        specific files, to entries in those files. Without the second filter a
        scoped `arc verify` declares the whole baseline stale simply because it
        linted a different file.
        """
        out = []
        for i, (tool, file, line, why) in enumerate(self.entries):
            if i in self.used or tool not in ran_tools:
                continue
            if scope_files is not None and file not in scope_files:
                continue
            out.append(f"{file}:{line}: {tool} no longer fires"
                       + (f" ({why})" if why else ""))
        return out


# --------------------------------------------------------------------------
# capabilities -- probed once, cached
# --------------------------------------------------------------------------

class Caps:
    def __init__(self, required: set[str] | None = None):
        self._cache: dict[str, tuple[bool, str]] = {}
        self.required = required or set()

    def _probe(self, name: str) -> tuple[bool, str]:
        if name == "src":
            return (REPO / "src").is_dir(), "src/ missing"
        if name == "exe":
            if not Path(os.environ.get("IWD2_EXE", str(REPO / ".bin" / "iwd2.exe"))).exists():
                return False, ".bin/iwd2.exe missing"
            rc = subprocess.run([PY, "-c", "import pefile"],
                                capture_output=True).returncode
            return rc == 0, "pefile not importable (need .venv-reagent)"
        if name == "exports":
            return (EXPORTS / "_index.json").exists(), ".ghidra-exports/ missing"
        if name == "cppcheck":
            return bool(cppcheck_executable()), "cppcheck missing (PATH, IWD2_CPPCHECK, or .tools/cppcheck)"
        if name == "ghidra":
            if not (REPO / "re-agent.host.yaml").exists():
                return False, "re-agent.host.yaml missing"
            # The bridge exports headlessly against the same project the GUI
            # locks. Probing here is what keeps arc from ever racing it.
            if WINDOWS:
                from native import powershell
                gui = powershell("$p = Get-CimInstance Win32_Process -Filter \"Name='java.exe' OR Name='javaw.exe'\" | "
                                 "Where-Object { $_.CommandLine -match 'ghidra.GhidraRun|ghidraRun' }; "
                                 "if ($p) { exit 0 } else { exit 1 }")
            else:
                gui = subprocess.run(["pgrep", "-f", "ghidra.GhidraRun|ghidraRun"],
                                     capture_output=True)
            if gui.returncode == 0:
                return False, "Ghidra GUI is open (close it first)"
            if not venv_tool("re-agent").is_file():
                return False, "re-agent is not installed in .venv-reagent"
            return True, ""
        if name == "vm":
            if WINDOWS:
                # Historical --require vm now means the local runtime adapter.
                return (SCRIPTS / "native.py").is_file(), "native Windows adapter missing"
            # The VM's default ssh shell is PowerShell, so `true` is not a
            # command there -- probing with it reports every VM as unreachable.
            rc = subprocess.run(
                ["ssh", "-o", "BatchMode=yes", "-o", "ConnectTimeout=6",
                 "win11vm", "exit 0"], capture_output=True, timeout=30).returncode
            return rc == 0, "win11vm unreachable"
        return False, f"unknown capability {name}"

    def has(self, name: str) -> bool:
        return self.check(name)[0]

    def check(self, name: str) -> tuple[bool, str]:
        if name not in self._cache:
            self._cache[name] = self._probe(name)
        return self._cache[name]

    def missing(self, names) -> str | None:
        """First unmet capability's reason, or None if all are met."""
        for n in names:
            ok, why = self.check(n)
            if not ok:
                return why
        return None


# --------------------------------------------------------------------------
# adapters -- tool stdout -> Report
#
# These exist so arc works today, against tools that print prose and signal
# through exit codes. When a tool grows --json its adapter is deleted.
# --------------------------------------------------------------------------

# Every lint prints `src/path.cpp:123: message`, with optional indented detail
# lines beneath. One adapter covers all seven. The line number is optional:
# a whole-file finding (an unbalanced pack at EOF) has no single line to blame.
HIT_RE = re.compile(
    r"^(?P<file>(?:src|scripts|refs)[\\/]\S+?)(?::(?P<line>\d+))?:\s*(?P<msg>\S.*)$")
RE_LINT_CODE_RE = re.compile(r"^(?P<code>RE\d{3}):\s*(?P<msg>.*)$")


def parse_lint(tool: str, rc: int, out: str, err: str) -> Report:
    rep = Report(tool=tool, exit_code=rc)
    for line in out.splitlines():
        m = HIT_RE.match(line)
        if not m:
            continue
        msg = m["msg"]
        code = None
        cm = RE_LINT_CODE_RE.match(msg)
        if cm:
            code, msg = cm["code"], cm["msg"]
        rep.findings.append(Finding(
            severity="fail", message=msg, file=m["file"].replace("\\", "/"), code=code,
            line=int(m["line"]) if m["line"] else None))
    if rc != 0 and not rep.findings:
        # Exited non-zero with nothing parseable: a crash, not a finding.
        rep.status = "error"
        rep.summary = (err.strip().splitlines() or ["failed with no parseable output"])[-1][:120]
    return rep


def parse_vtable_audit(rc: int, out: str, err: str) -> Report:
    rep = Report(tool="vtable_audit", exit_code=rc)
    cur_class = None
    for line in out.splitlines():
        mc = re.match(r"^=== \S+ : (?P<cls>\w+)\s+vtable", line)
        if mc:
            cur_class = mc["cls"]
            continue
        m = re.match(r"^\s*\[(?P<kind>\w+)\s*\] (?P<rest>.*)$", line)
        if m:
            rep.findings.append(Finding(
                severity="fail" if m["kind"] in ("MISSING", "WRONGADDR") else "warn",
                code=m["kind"], symbol=cur_class, message=m["rest"].strip()))
    mt = re.search(r"^TOTAL findings: (\d+)$", out, re.M)
    if mt:
        rep.counts["findings"] = int(mt.group(1))
    mp = re.search(r"^parsed (\d+) classes.*$", out, re.M)
    if mp:
        rep.notes.append(mp.group(0))
        rep.counts["scanned"] = int(mp.group(1))
    return rep


def parse_ctor_vtable(rc: int, out: str, err: str) -> Report:
    rep = Report(tool="ctor_vtable_check", exit_code=rc)
    for line in out.splitlines():
        if "CONFLATION" in line:
            rep.findings.append(Finding(severity="fail", code="CONFLATION",
                                        message=line.strip()))
    m = re.search(r"(\d+) class\(es\) with ctors scanned; (\d+) conflation", out)
    if m:
        rep.counts["scanned"] = int(m.group(1))
        rep.counts["findings"] = int(m.group(2))
        rep.summary = f"{m.group(1)} classes, {m.group(2)} conflation(s)"
    return rep


def parse_parity_offsets(rc: int, out: str, err: str) -> Report:
    rep = Report(tool="parity_offsets", exit_code=rc)
    for line in out.splitlines():
        if re.search(r"\bMISMATCH\b|\bWRONG\b|\bno source member\b", line):
            rep.findings.append(Finding(severity="fail", message=line.strip()))
    if rc not in (0, 1, 2):
        rep.status = "error"
    return rep


def parse_arg_provenance(rc: int, out: str, err: str) -> Report:
    """--sweep prints an `== check:` roll-up; a single address prints only the
    per-site rows, so the counts have to come from the tags either way."""
    rep = Report(tool="arg_provenance", exit_code=rc)
    ok = review = swap = 0
    for line in out.splitlines():
        s = line.strip()
        if s.startswith("== check"):
            continue
        if "SWAP?" in s:
            swap += 1
            rep.findings.append(Finding(severity="fail", message=s))
        elif s.endswith(" review"):
            review += 1
            rep.findings.append(Finding(severity="warn", message=s))
        elif s.endswith(" OK"):
            ok += 1
        elif "STUB/inlined" in s or "COUNT MISMATCH" in s:
            review += 1
            rep.findings.append(Finding(severity="warn", message=s))

    m = re.search(r"== check: (\d+) OK, (\d+) review, (\d+) SWAP\?", out)
    if m:
        ok, review, swap = (int(x) for x in m.groups())
    rep.counts.update(ok=ok, review=review, swap=swap)
    total = ok + review + swap
    if swap:
        rep.summary = f"{swap} suspected operand SWAP of {total} site(s)"
    elif review:
        rep.summary = f"{total} operator+ sites: {ok} OK, {review} to review"
    else:
        rep.summary = f"{total} operator+ sites: all OK" if total else "no operator+ sites"
    return rep


def parse_parity_sweep(rc: int, out: str, err: str) -> Report:
    rep = Report(tool="parity_cache_sweep", exit_code=rc)
    m = re.search(r"Swept (\d+) hooks \((\d+) with cached decompile\): RED=(\d+) YELLOW=(\d+)", out)
    if m:
        swept, cached, red, yellow = (int(x) for x in m.groups())
        rep.counts.update(scanned=swept, cached=cached, red=red, yellow=yellow)
        rep.summary = f"{swept} hooks: RED={red} YELLOW={yellow}"
    for line in out.splitlines():
        ml = re.match(r"^\s*\[(RED|YELLOW)\s*\] (?P<sym>\S+)", line)
        if ml:
            rep.findings.append(Finding(
                severity="fail" if ml.group(1) == "RED" else "warn",
                symbol=ml.group("sym"), message=line.strip()))
    return rep


def parse_struct_layout(rc: int, out: str, err: str) -> Report:
    """struct_layout_audit exits 1 both for real drift and for its own error
    paths (sys.exit("[audit] class X not found in ...")). Exit code alone cannot
    tell them apart, so disambiguate on stdout: a run that reached a verdict
    always prints `[audit] PASS` or `[audit] RESULT:`. Neither means nothing was
    audited -- a harness error, which must never render as a red verdict."""
    rep = Report(tool="struct_layout_audit", exit_code=rc)
    verdict = re.search(r"^\[audit\] (?P<kind>PASS\b|RESULT:)(?P<body>.*)$", out, re.M)
    if not verdict:
        rep.status = "error"
        tail = (err.strip() or out.strip()).splitlines()
        rep.summary = tail[-1][:120] if tail else "no verdict line"
        return rep
    body = re.sub(r"^[-\s]+", "", verdict["body"]).strip()
    rep.summary = ("PASS: " + body if verdict["kind"] == "PASS" else body)[:70]
    if "-> FAIL" in body:
        rep.status = "fail"
        for line in out.splitlines():
            if re.search(r"ACTIONABLE|OUT-OF-ORDER", line):
                rep.findings.append(Finding(severity="fail", message=line.strip()))
    return rep


def parse_parity_json(rc: int, out: str, err: str, path: Path) -> Report:
    """re-agent parity --output already writes a clean per-finding JSON report,
    so the verdict comes from the file, never from stdout."""
    rep = Report(tool="parity", exit_code=rc)
    if not path.exists():
        rep.status = "error"
        tail = (err.strip() or out.strip()).splitlines()
        rep.summary = tail[-1][:160] if tail else "no parity report written"
        return rep
    try:
        data = json.loads(path.read_text(encoding="utf-8"))
    except (json.JSONDecodeError, OSError) as exc:
        rep.status = "error"
        rep.summary = f"unreadable parity report: {exc}"
        return rep
    worst = "green"
    rank = {"green": 0, "yellow": 1, "red": 2}
    for r in data.get("results", []):
        st = (r.get("status") or "green").lower()
        if rank.get(st, 0) > rank[worst]:
            worst = st
        for f in r.get("findings", []):
            level = (f.get("level") or "yellow").lower()
            if level in ("red", "yellow"):
                rep.findings.append(Finding(
                    severity="fail" if level == "red" else "warn",
                    symbol=r.get("symbol"), address=r.get("address"),
                    message=f.get("reason", "")))
    rep.summary = worst.upper()
    rep.status = {"green": "pass", "yellow": "warn", "red": "fail"}[worst]
    return rep


def parse_build(rc: int, out: str, err: str) -> Report:
    rep = Report(tool="build", exit_code=rc)
    if rc == 0:
        rep.summary = "BUILD OK"
        return rep
    rep.status = "fail"
    m = re.search(r"BUILD FAILED \((\d+) errors\)", out)
    rep.summary = m.group(0) if m else "BUILD FAILED"
    for line in out.splitlines():
        if re.search(r": (fatal )?error [A-Z]*\d+|LNK\d+", line):
            rep.findings.append(Finding(severity="fail", message=line.strip()))
    return rep


def parse_smoke(rc: int, out: str, err: str) -> Report:
    """vm.sh smoke: 0 = clean, 1 = crash, 2 = ran clean but the path never
    executed (NOT-EXERCISED / NOT-INSTRUMENTED). 2 is a warning, not a pass:
    an idle 90s proves nothing about recovered code."""
    rep = Report(tool="smoke", exit_code=rc)
    # vm.sh banners the verdict inside a rule of '=' -- match anywhere on the line.
    m = re.search(r"RESULT: (?P<v>[A-Z][A-Z-]+)", out)
    verdict = m["v"] if m else ("CLEAN" if rc == 0 else "UNKNOWN")
    rep.summary = verdict
    mh = re.search(r"\bhit x(\d+)", out)
    if mh:
        rep.summary = f"{verdict}, hit x{mh.group(1)}"
        rep.counts["hits"] = int(mh.group(1))
    if rc == 1:
        rep.status = "fail"
        for line in out.splitlines():
            if re.search(r"EXCEPTION|^\s+\d+ ", line):
                rep.findings.append(Finding(severity="fail", message=line.strip()))
    elif rc == 2:
        rep.status = "warn"
        rep.findings.append(Finding(
            severity="warn",
            message=f"{verdict}: build did not crash, but the recovered path "
                    f"never ran -- this proves nothing"))
    elif rc != 0:
        rep.status = "error"
    return rep


# --------------------------------------------------------------------------
# step execution
# --------------------------------------------------------------------------

@dataclass
class Step:
    id: str
    label: str
    argv: list[str]
    parse: object
    needs: tuple = ()
    cwd: Path = REPO
    timeout: int = 600
    skip_reason: str | None = None
    # Advisory steps report a count and can never fail the run. Used for the
    # whole-tree passes of re_lint (1600+ standing hits) and lint_uninit_member,
    # which are periodic audits: failing on them would gate every commit on debt
    # the commit did not create.
    advisory: bool = False


class Runner:
    def __init__(self, caps: Caps, baseline: Baseline, rundir: Path, progress=False):
        self.caps = caps
        self.baseline = baseline
        self.rundir = rundir
        # A sweep runs for minutes. Without this it prints nothing at all until
        # the very end, which is indistinguishable from a hang -- and one step
        # (parity_cache_sweep) really can run for 10+ minutes. Goes to stderr so
        # it never pollutes --json or the --quiet hook path.
        self.progress = progress
        self.ran_tools: set[str] = set()
        self._done = 0
        self._inflight: dict[str, float] = {}
        self._lock = threading.Lock()
        self._heartbeat_started = False

    # Only slow or failing steps report on completion. A sub-5s step needs no
    # reassurance, and ticking all of them would put a dozen lines of noise in
    # front of a verdict whose whole point is being short.
    TICK_AFTER_S = 5.0
    # A step is "slow" once it has been in flight this long; the heartbeat then
    # names it every HEARTBEAT_EVERY_S. Completion ticks alone are not enough:
    # they fire only when a step ENDS, so parity_cache_sweep (488s measured on
    # the full tree) still produced eight minutes of total silence.
    HEARTBEAT_AFTER_S = 20.0
    HEARTBEAT_EVERY_S = 30.0

    def _start_heartbeat(self) -> None:
        if self._heartbeat_started or not self.progress:
            return
        self._heartbeat_started = True

        def beat():
            while True:
                time.sleep(self.HEARTBEAT_EVERY_S)
                now = time.monotonic()
                with self._lock:
                    slow = [(lbl, now - t) for lbl, t in self._inflight.items()
                            if now - t >= self.HEARTBEAT_AFTER_S]
                if slow:
                    parts = ", ".join(f"{l} {d:.0f}s" for l, d in sorted(slow))
                    print(f"  ... still running: {parts}", file=sys.stderr, flush=True)

        threading.Thread(target=beat, daemon=True).start()

    def _tick(self, rep: Report) -> None:
        if not self.progress:
            return
        self._done += 1
        if rep.elapsed_s < self.TICK_AFTER_S and rep.status not in ("fail", "error"):
            return
        print(f"  [{self._done}] {rep.label or rep.tool}: {rep.status} "
              f"({rep.elapsed_s:.0f}s) {rep.summary[:60]}",
              file=sys.stderr, flush=True)

    def run(self, step: Step) -> Report:
        self._start_heartbeat()
        label = step.label or step.id
        with self._lock:
            self._inflight[label] = time.monotonic()
        try:
            rep = self._run(step)
        finally:
            with self._lock:
                self._inflight.pop(label, None)
        self._tick(rep)
        return rep

    def _run(self, step: Step) -> Report:
        if step.skip_reason:
            return Report(tool=step.id, status="skip", summary=step.skip_reason,
                          label=step.label)
        why = self.caps.missing(step.needs)
        if why:
            required = bool(self.caps.required & set(step.needs))
            return Report(tool=step.id, status="fail" if required else "skip",
                          summary=why, label=step.label)

        t0 = time.monotonic()
        try:
            proc = subprocess.run(step.argv, cwd=step.cwd, capture_output=True,
                                  encoding="utf-8", errors="replace", timeout=step.timeout)
            rc, out, err = proc.returncode, proc.stdout, proc.stderr
        except subprocess.TimeoutExpired:
            rep = Report(tool=step.id, status="error", label=step.label,
                         summary=f"timed out after {step.timeout}s")
            rep.elapsed_s = time.monotonic() - t0
            return rep
        except (OSError, FileNotFoundError) as exc:
            rep = Report(tool=step.id, status="error", label=step.label,
                         summary=str(exc)[:120])
            rep.elapsed_s = time.monotonic() - t0
            return rep

        self._capture(step.id, out, err)
        rep = step.parse(rc, out, err)
        rep.label = step.label
        rep.advisory = step.advisory
        rep.elapsed_s = time.monotonic() - t0
        self.ran_tools.add(rep.tool)

        n_base = self.baseline.apply(rep.tool, rep.findings)
        if n_base:
            rep.counts["baselined"] = n_base
        live = [f for f in rep.findings if not f.baselined]

        if rep.status not in ("error", "skip"):
            if any(f.severity == "fail" for f in live):
                rep.status = "fail"
            elif any(f.severity == "warn" for f in live):
                rep.status = "warn"
            elif rep.status == "pass" and rc not in (0, None) and not rep.findings:
                # Non-zero with everything baselined away is still a pass.
                rep.status = "pass" if n_base else rep.status
        if not rep.summary:
            n = len(live)
            rep.summary = f"{n} finding(s)" if n else "clean"
            if n_base:
                rep.summary += f" ({n_base} baselined)"
        if step.advisory and rep.status == "fail":
            rep.status = "warn"
            rep.summary = f"{len(live)} pre-existing hit(s), advisory"
        return rep

    def _capture(self, step_id: str, out: str, err: str) -> None:
        self.rundir.mkdir(parents=True, exist_ok=True)
        if out:
            (self.rundir / f"{step_id}.out").write_text(out, encoding="utf-8")
        if err:
            (self.rundir / f"{step_id}.err").write_text(err, encoding="utf-8")

    def run_parallel(self, steps: list[Step]) -> list[Report]:
        """Order out == order in; the tools are independent read-only processes."""
        if len(steps) == 1:
            return [self.run(steps[0])]
        with concurrent.futures.ThreadPoolExecutor(max_workers=min(8, len(steps))) as ex:
            return list(ex.map(self.run, steps))


# --------------------------------------------------------------------------
# rendering
# --------------------------------------------------------------------------

def verdict_of(reports: list[Report], strict: bool = False) -> tuple[str, int]:
    fails = [r for r in reports if r.status == "fail"]
    errors = [r for r in reports if r.status == "error"]
    warns = [r for r in reports if r.status == "warn"]
    if strict:
        fails = fails + warns
        warns = []
    if errors and not fails:
        return "ERROR", 2
    if fails:
        return "FAIL", 1
    if warns:
        return "PASS with " + (f"{len(warns)} warning" + ("s" if len(warns) > 1 else "")), 0
    return "PASS", 0


def render(reports: list[Report], header: str, elapsed: float,
           max_findings: int, baseline: Baseline, ran: set[str],
           strict: bool = False, extra: list[str] | None = None,
           scope_files: set[str] | None = None) -> int:
    label, code = verdict_of(reports, strict)
    if header:
        print(header)
    for r in reports:
        dur = f"{r.elapsed_s:5.1f}s" if r.elapsed_s else "     -"
        print(f"  {TAG[r.status]} {r.label:<13} {r.summary[:52]:<52} {dur}")
    n_skip = sum(1 for r in reports if r.status == "skip")
    n_fail = sum(1 for r in reports if r.status in ("fail", "error"))
    tail = f"({len(reports)} steps"
    if n_skip:
        tail += f", {n_skip} skipped"
    if n_fail:
        tail += f", {n_fail} failure" + ("s" if n_fail > 1 else "")
    tail += f", {fmt_dur(elapsed)})"
    print(f"VERDICT: {label}   {tail}")

    # Advisory steps report a count only: listing 1600 standing hits would bury
    # the handful of findings this run actually produced.
    detail = [r for r in reports if not r.advisory]
    shown = 0
    for r in detail:
        for f in r.findings:
            if f.baselined or shown >= max_findings:
                continue
            loc = f.file or f.symbol or f.address or ""
            if f.file and f.line:
                loc = f"{f.file}:{f.line}"
            print(f"  {r.label:<12} {loc}  {f.message[:100]}".rstrip())
            shown += 1
    hidden = sum(len([f for f in r.findings if not f.baselined]) for r in detail) - shown
    if hidden > 0:
        print(f"  ... {hidden} more finding(s) -- scripts/arc.py explain <step>")

    for msg in baseline.stale(ran, scope_files):
        print(f"  WARN stale baseline: {msg}")
    for line in (extra or []):
        print(line)
    return code


def fmt_dur(s: float) -> str:
    return f"{s:.1f}s" if s < 60 else f"{int(s // 60)}m{int(s % 60):02d}s"


def write_run_json(rundir: Path, payload: dict) -> None:
    rundir.mkdir(parents=True, exist_ok=True)
    (rundir / "run.json").write_text(json.dumps(payload, indent=2), encoding="utf-8")
    last = ARCDIR / "last"
    try:
        if last.is_symlink() or last.exists():
            last.unlink()
        last.symlink_to(rundir.name)
    except OSError:
        (ARCDIR / "last.txt").write_text(rundir.name, encoding="utf-8")


def new_rundir(cmd: str) -> Path:
    stamp = datetime.now().strftime("%Y-%m-%dT%H-%M-%S")
    return ARCDIR / f"{stamp}-{cmd}"


# --------------------------------------------------------------------------
# lint battery (shared by check and verify)
# --------------------------------------------------------------------------

def lint_steps(scope_file: str | None, mode: str) -> list[Step]:
    """mode: 'staged' | 'all' | 'file'.

    re_lint and lint_uninit_member are changed-lines-only by design and stay
    that way: re_lint --all reports 1600+ pre-existing hits, which is an audit,
    not a gate.
    """
    def s(sid, label, args, advisory=False):
        needs = ("src",)
        if sid == "lint_uninit_member":
            needs += ("cppcheck",)
        elif sid == "lint_address_markers":
            needs += ("exports",)
        return Step(id=sid, label=label, parse=lambda rc, o, e, t=sid: parse_lint(t, rc, o, e),
                    argv=[PY, str(SCRIPTS / f"{sid}.py")] + args,
                    needs=needs, timeout=600, advisory=advisory)

    if mode == "file" and scope_file:
        # re_lint and lint_uninit_member stay on the CHANGED LINES even here.
        # Pointed at a whole recovered file they report its standing debt
        # (136 hits in CGameSprite.cpp) -- none of it authored by this arc, and
        # burying the arc's own findings under it defeats the verdict.
        # The structural lints are whole-file and currently tree-wide clean.
        return [
            s("re_lint", "re_lint", ["--staged", "--worktree"]),
            s("lint_pragma_pack_balance", "pragma", []),
            s("lint_twin_symmetry", "twin", ["--file", scope_file, "--quiet"]),
            s("lint_extend_cells", "extend", [scope_file, "--quiet"]),
            s("lint_infinite_loop", "infloop", [scope_file, "--quiet"]),
            s("lint_stuck_loop_index", "stuckidx", [scope_file, "--quiet"]),
            s("lint_uninit_member", "uninit", ["--quiet"]),
            s("lint_address_markers", "markers", ["--quiet"]),
        ]
    audit = mode == "all"
    return [
        s("re_lint", "re_lint", ["--all"] if audit else ["--staged"], advisory=audit),
        s("lint_pragma_pack_balance", "pragma", []),
        s("lint_twin_symmetry", "twin", ["--quiet"]),
        s("lint_extend_cells", "extend", ["--quiet"]),
        s("lint_infinite_loop", "infloop", ["--quiet"]),
        s("lint_stuck_loop_index", "stuckidx", ["--quiet"]),
        s("lint_uninit_member", "uninit", ["--all"] if audit else ["--quiet"],
          advisory=audit),
        s("lint_address_markers", "markers", ["--quiet"]),
    ]


def git(*args: str) -> str:
    try:
        return subprocess.run(["git", *args], cwd=REPO, capture_output=True,
                              text=True, timeout=30).stdout
    except (OSError, subprocess.TimeoutExpired):
        return ""


def staged_files() -> list[str]:
    out = git("diff", "--cached", "--name-only", "--diff-filter=ACMR")
    return [l for l in out.splitlines() if l.startswith("src/")]


# --------------------------------------------------------------------------
# cmd: check
# --------------------------------------------------------------------------

def cmd_check(args) -> int:
    t0 = time.monotonic()
    mode = "all" if args.all else "staged"
    files = staged_files()
    if mode == "staged" and not files and not args.all:
        # Nothing of ours staged: the whole-tree lints would still run and their
        # verdict would be about code this commit does not touch.
        if not args.json and not args.quiet:
            print("arc check: no staged src/ files -- nothing to gate")
        return 0

    caps = Caps(set(args.require or []))
    baseline = Baseline()
    rundir = new_rundir("check")
    runner = Runner(caps, baseline, rundir)
    reports = runner.run_parallel(lint_steps(None, mode))
    elapsed = time.monotonic() - t0

    extra = advisory_pack2(files) if mode == "staged" else []
    payload = {"schema": SCHEMA, "cmd": "check", "mode": mode,
               "files": files, "elapsed_s": round(elapsed, 2),
               "verdict": verdict_of(reports, args.strict)[0],
               "steps": [r.to_json() for r in reports]}
    write_run_json(rundir, payload)

    if args.json:
        print(json.dumps(payload, indent=2))
        return verdict_of(reports, args.strict)[1]

    code = verdict_of(reports, args.strict)[1]
    if args.quiet and code == 0 and not extra:
        return 0
    header = (f"arc check  ({mode}: {len(files)} file(s))" if mode == "staged"
              else "arc check  (whole tree)")
    code = render(reports, header, elapsed, args.max_findings, baseline,
                  runner.ran_tools, args.strict, extra)
    if code != 0:
        print("retry: scripts/arc.py check      bypass: ARC_SKIP=1 git commit ...")
    return code


def advisory_pack2(files: list[str]) -> list[str]:
    """A header gaining /* 0xNNN */ offset comments without #pragma pack(2) is
    the Cloudkill bug class: MSVC 4-aligns a sub-4-aligned tail, parity stays
    green (it is layout-blind) and only a visual bug ever shows. Non-blocking:
    a legitimate non-mirror header can carry offset comments."""
    out = []
    for f in files:
        if not f.endswith(".h"):
            continue
        p = REPO / f
        if not p.exists():
            continue
        try:
            text = p.read_text(encoding="utf-8", errors="replace")
        except OSError:
            continue
        if "/* 0x" in text and "pragma pack(2)" not in text.replace(" ", " "):
            if not re.search(r"#pragma\s+pack\s*\(\s*(push\s*,\s*)?2\s*\)", text):
                out.append(f"  ADVISORY: {f} is a binary-mirror header without "
                           f"#pragma pack(2) -- verify with struct_layout_audit")
    return out


# --------------------------------------------------------------------------
# cmd: verify
# --------------------------------------------------------------------------

ADDR_RE = re.compile(r"^0x[0-9A-Fa-f]{5,8}$")


def resolve_address(addr: str) -> dict:
    """address -> {file, line, symbol, class}. git grep, not src_index.json:
    a 3ms grep of the working tree cannot be stale, a cache can."""
    norm = addr.lower()
    bare = norm[2:].lstrip("0")
    info: dict = {"address": norm, "file": None, "line": None,
                  "symbol": None, "class": None}
    out = git("grep", "-n", "-i", "-E", rf"^\s*//\s*0x0*{bare}\b", "--", "src")
    if not out.strip():
        return info
    first = out.splitlines()[0]
    parts = first.split(":", 2)
    if len(parts) < 2:
        return info
    info["file"], info["line"] = parts[0], int(parts[1])

    path = REPO / info["file"]
    try:
        lines = path.read_text(encoding="utf-8", errors="replace").splitlines()
    except OSError:
        return info
    # The marker sits above the definition; scan a few lines for the signature.
    for ln in lines[info["line"]:info["line"] + 6]:
        s = ln.strip()
        if not s or s.startswith("//") or s.startswith("/*"):
            continue
        m = re.search(r"(?P<cls>\w+)::(?P<fn>~?\w+)\s*\(", s)
        if m:
            info["class"], info["symbol"] = m["cls"], f"{m['cls']}::{m['fn']}"
        else:
            m2 = re.search(r"\b(?P<fn>\w+)\s*\(", s)
            if m2:
                info["symbol"] = m2["fn"]
        break
    return info


def ghidra_name(addr: str) -> str | None:
    idx = EXPORTS / "_index.json"
    if not idx.exists():
        return None
    try:
        data = json.loads(idx.read_text(encoding="utf-8"))
    except (json.JSONDecodeError, OSError):
        return None
    key = f"{int(addr, 16):08x}"
    entry = data.get(key) or data.get(key.upper())
    return entry.get("name") if isinstance(entry, dict) else None


def header_for_class(cls: str) -> Path | None:
    direct = REPO / "src" / f"{cls}.h"
    if direct.exists():
        return direct
    out = git("grep", "-l", "-E", rf"^\s*(class|struct)\s+{cls}\b", "--", "src/*.h")
    for line in out.splitlines():
        return REPO / line
    return None


def cmd_verify(args) -> int:
    t0 = time.monotonic()
    addr = args.address.lower()
    if not ADDR_RE.match(addr):
        print(f"arc verify: '{args.address}' is not a 0xADDR", file=sys.stderr)
        return 2

    info = resolve_address(addr)
    if not info["file"]:
        print(f"arc verify: {addr} has no // {addr} marker in src/ "
              f"-- not recovered yet")
        print("            see: scripts/arc.py targets")
        return 2

    caps = Caps(set(args.require or []))
    baseline = Baseline()
    rundir = new_rundir("verify")
    runner = Runner(caps, baseline, rundir, progress=not args.json)
    cls, sym = info["class"], info["symbol"]
    notes: list[str] = []

    gname = ghidra_name(addr)
    if gname and sym and gname != sym and not gname.startswith("FUN_"):
        notes.append(f"  WARN name mismatch: source says {sym}, Ghidra says {gname}")

    reports: list[Report] = []

    # phase 1 -- lints on the touched file, folded into one row
    steps = lint_steps(info["file"], "file")
    lints = runner.run_parallel(steps)
    reports.append(fold(lints, "lint",
                        f"{len(steps)} checks on {Path(info['file']).name}"))

    # phase 2 -- binary oracles, parallel
    bin_steps: list[Step] = [
        Step(id="parity_offsets", label="parity_off", needs=("exe",), timeout=300,
             argv=[PY, str(SCRIPTS / "parity_offsets.py"), addr],
             parse=parse_parity_offsets),
        Step(id="arg_provenance", label="arg_prov", needs=("exe",), timeout=300,
             argv=[PY, str(SCRIPTS / "arg_provenance.py"), addr, "--check"],
             parse=parse_arg_provenance),
    ]
    if cls:
        bin_steps += [
            Step(id="ctor_vtable_check", label="ctor_vtable", needs=("exe",), timeout=300,
                 argv=[PY, str(SCRIPTS / "ctor_vtable_check.py"), cls, "--quiet"],
                 parse=parse_ctor_vtable),
            Step(id="vtable_audit", label="vtable", needs=("exe",), timeout=300,
                 argv=[PY, str(SCRIPTS / "vtable_audit.py"), cls, "--quiet"],
                 parse=parse_vtable_audit),
        ]
    else:
        for sid, label in (("ctor_vtable_check", "ctor_vtable"), ("vtable_audit", "vtable")):
            reports.append(Report(tool=sid, status="skip", label=label,
                                  summary="free function (no class to audit)"))
    reports += runner.run_parallel(bin_steps)

    # phase 3 -- parity, strictly serial (one PyGhidra per project)
    if not args.no_parity:
        pj = rundir / "parity.json"
        pargv = [str(venv_tool("re-agent")),
                 "--config", str(REPO / "re-agent.host.yaml"),
                 "parity", "--address", addr, "--output", str(pj)]
        reports.append(runner.run(Step(
            id="parity", label="parity", needs=("ghidra",), timeout=900,
            argv=pargv, parse=lambda rc, o, e: parse_parity_json(rc, o, e, pj))))

    # phase 4 -- struct layout, only for a real binary-mirror class
    reports.append(runner.run(layout_step(cls)))

    # phase 5 -- build (the only fail-fast boundary)
    build_rep = None
    if not args.no_build:
        build_rep = runner.run(Step(id="build", label="build", needs=("vm",),
                                    timeout=1200, parse=parse_build,
                                    argv=runtime_command("build")))
        reports.append(build_rep)

    # phase 6 -- smoke; meaningless if the build failed
    if not args.no_smoke:
        if build_rep is not None and build_rep.status == "fail":
            reports.append(Report(tool="smoke", status="skip", label="smoke",
                                  summary="build failed -- smoke would prove nothing"))
        else:
            smoke_argv = runtime_command("smoke", str(args.slot), str(args.hold))
            if WINDOWS and args.new_game:
                smoke_argv += ["--new-game", "--party", args.party]
            if args.save:
                smoke_argv += ["--save", args.save]
            if args.ui:
                smoke_argv += ["--ui", args.ui]
            hit = args.hit or sym
            if args.expect:
                smoke_argv += ["--expect", args.expect]
            elif hit:
                smoke_argv += ["--hit", hit]
            reports.append(runner.run(Step(
                id="smoke", label="smoke", needs=("vm",), timeout=args.hold + 600,
                argv=smoke_argv, parse=parse_smoke)))

    elapsed = time.monotonic() - t0
    smoke = next((r for r in reports if r.tool == "smoke"), None)
    if smoke and smoke.status == "pass" and "hit x" not in smoke.summary:
        notes.append("  NOTE runtime unproven: smoke saw no fault, but nothing "
                     "proved the recovered path executed")
    if touches_visual(info["file"], cls, sym):
        notes.append(f"VISUAL: user check required ({cls or info['file']} is on a "
                     f"render path) -> scripts/spell_capture/spellcap.sh compare <Spell>")

    payload = {"schema": SCHEMA, "cmd": "verify", "address": addr,
               "symbol": sym, "class": cls, "file": info["file"],
               "line": info["line"], "elapsed_s": round(elapsed, 2),
               "verdict": verdict_of(reports, args.strict)[0],
               "steps": [r.to_json() for r in reports]}
    write_run_json(rundir, payload)
    if args.json:
        print(json.dumps(payload, indent=2))
        return verdict_of(reports, args.strict)[1]

    header = (f"arc verify {addr}   {sym or '?'}   "
              f"{info['file']}:{info['line']}")
    notes.append(f"detail: scripts/arc.py explain <step>    logs: .arc/{rundir.name}/")
    return render(reports, header, elapsed, args.max_findings, baseline,
                  runner.ran_tools, args.strict, notes,
                  scope_files={info["file"]})


def layout_step(cls: str | None) -> Step:
    """struct_layout_audit needs a class, and only means something for a
    binary-mirror class. Derive; never guess."""
    if not cls:
        return Step(id="struct_layout_audit", label="layout", argv=[], parse=parse_struct_layout,
                    skip_reason="free function (no class)")
    hdr = header_for_class(cls)
    if hdr is None:
        return Step(id="struct_layout_audit", label="layout", argv=[], parse=parse_struct_layout,
                    skip_reason=f"no header declares {cls}")
    try:
        text = hdr.read_text(encoding="utf-8", errors="replace")
    except OSError:
        text = ""
    if "/* 0x" not in text:
        return Step(id="struct_layout_audit", label="layout", argv=[], parse=parse_struct_layout,
                    skip_reason=f"{hdr.name} has no /* 0xNNN */ comments")
    return Step(id="struct_layout_audit", label="layout", needs=("vm",), timeout=600,
                argv=[PY, str(SCRIPTS / "struct_layout_audit.py"), cls],
                parse=parse_struct_layout)


def touches_visual(file: str | None, cls: str | None, sym: str | None) -> bool:
    hay = " ".join(x for x in (file, cls, sym) if x)
    return any(p in hay for p in VISUAL_PATTERNS)


def fold(reports: list[Report], tool: str, summary: str) -> Report:
    """Collapse a battery into one row; the detail stays in .arc/<run>/."""
    out = Report(tool=tool, label=tool)
    for r in reports:
        out.findings.extend(r.findings)
        out.notes.extend(r.notes)
        if STATUS_ORDER[r.status] < STATUS_ORDER[out.status]:
            out.status = r.status
        if r.status in ("skip", "error"):
            out.notes.append(f"{r.tool}: {r.summary}")
    out.elapsed_s = max((r.elapsed_s for r in reports), default=0.0)
    live = [f for f in out.findings if not f.baselined]
    out.summary = f"{summary}, {len(live)} hit(s)" if live else f"{summary}, 0 hits"
    skipped = sum(r.status == "skip" for r in reports)
    if skipped:
        out.summary += f", {skipped} skipped"
    return out


# --------------------------------------------------------------------------
# cmd: sweep / status / targets / explain / hooks
# --------------------------------------------------------------------------

def cmd_sweep(args) -> int:
    t0 = time.monotonic()
    caps = Caps(set(args.require or []))
    baseline = Baseline()
    rundir = new_rundir("sweep")
    runner = Runner(caps, baseline, rundir, progress=not args.json)

    reports = runner.run_parallel(lint_steps(None, "all"))
    heavy = [
        Step(id="parity_cache_sweep", label="parity_sw", needs=("exports", "ghidra"),
             timeout=1800, argv=[PY, str(SCRIPTS / "parity_cache_sweep.py")],
             parse=parse_parity_sweep),
        Step(id="vtable_audit", label="vtable", needs=("exe",), timeout=1800,
             argv=[PY, str(SCRIPTS / "vtable_audit.py"), "--quiet"],
             parse=parse_vtable_audit),
        Step(id="ctor_vtable_check", label="ctor_vtable", needs=("exe",), timeout=1800,
             argv=[PY, str(SCRIPTS / "ctor_vtable_check.py"), "--quiet"],
             parse=parse_ctor_vtable),
        Step(id="arg_provenance", label="arg_prov", needs=("exe",), timeout=1800,
             argv=[PY, str(SCRIPTS / "arg_provenance.py"), "--sweep", "--check"],
             parse=parse_arg_provenance),
    ]
    if args.deep:
        heavy.append(Step(
            id="parity_offsets", label="parity_off", needs=("exe",), timeout=7200,
            argv=[PY, str(SCRIPTS / "parity_offsets.py"), "--sweep"]
                 + sorted(str(p) for p in (REPO / "src").glob("*.cpp")),
            parse=parse_parity_offsets))
    else:
        reports.append(Report(tool="parity_offsets", status="skip", label="parity_off",
                              summary="whole-tree disasm -- pass --deep"))
    reports += runner.run_parallel(heavy)
    elapsed = time.monotonic() - t0

    stats = project_status_json()
    payload = {"schema": SCHEMA, "cmd": "sweep", "elapsed_s": round(elapsed, 2),
               "verdict": verdict_of(reports, args.strict)[0],
               "stats": stats, "steps": [r.to_json() for r in reports]}
    write_run_json(rundir, payload)

    if args.append_history and stats:
        append_history(payload)

    if args.json:
        print(json.dumps(payload, indent=2))
        return verdict_of(reports, args.strict)[1]
    return render(reports, "arc sweep  (whole codebase)", elapsed,
                  args.max_findings, baseline, runner.ran_tools, args.strict)


def project_status_json() -> dict:
    try:
        proc = subprocess.run([PY, str(SCRIPTS / "project_status.py"), "--json"],
                              cwd=REPO, capture_output=True, text=True, timeout=600)
        return json.loads(proc.stdout) if proc.returncode == 0 else {}
    except (OSError, subprocess.TimeoutExpired, json.JSONDecodeError):
        return {}


HISTORY = REPO / "docs" / "health-history.jsonl"


def append_history(payload: dict) -> None:
    row = {
        "date": datetime.now().strftime("%Y-%m-%d"),
        "commit": git("rev-parse", "--short", "HEAD").strip(),
        "verdict": payload.get("verdict"),
        "stats": payload.get("stats", {}),
        "steps": {s["tool"]: s["status"] for s in payload.get("steps", [])},
    }
    HISTORY.parent.mkdir(parents=True, exist_ok=True)
    with HISTORY.open("a", encoding="utf-8") as fh:
        fh.write(json.dumps(row) + "\n")
    print(f"appended: {HISTORY.relative_to(REPO)}")


def cmd_status(args) -> int:
    stats = project_status_json()
    if not stats:
        print("arc status: project_status.py --json failed "
              "(needs .ghidra-exports/ and .venv-reagent)")
        return 2
    if args.json:
        print(json.dumps(stats, indent=2))
        return 0

    for line in summarize_stats(stats, last_history()):
        print(line)
    for nag in freshness_nags():
        print(f"         {nag}")
    return 0


def summarize_stats(stats: dict, prev: dict | None) -> list[str]:
    src = stats.get("source", {})
    gh = stats.get("ghidra", {})
    by = stats.get("bytes", {})
    delta = ""
    if prev:
        old = (prev.get("stats") or {}).get("bytes", {}).get("pct")
        if isinstance(old, (int, float)) and isinstance(by.get("pct"), (int, float)):
            delta = f"  ({by['pct'] - old:+.1f}pt since {prev.get('date')})"
    out = [
        f"IWD2-RE  .text {by.get('pct')}% "
        f"({by.get('recovered_bytes', 0) / 1e6:.2f}/{by.get('text_bytes', 0) / 1e6:.2f} MB)"
        f"{delta}",
        f"         fns {src.get('recovered_funcs')}/{gh.get('total_funcs')} "
        f"({stats.get('recovery_pct')}%)   named {stats.get('named_pct')}%   "
        f"TODO/FIXME {src.get('todo_fixme')}   stubs {src.get('todo_incomplete')}",
    ]
    # The legacy line above divides by a denominator that is ~19.6k Unwind@/
    # Catch@ SEH funclets. Print the honest one next to it so nobody reads 37%
    # as "a third of the game". Same source as the legacy line, so the two
    # cannot drift apart.
    r = stats.get("real")
    if r:
        out.append(
            f"         real {r['recovered']:,}/{r['total']:,} ({r['pct']}%)   "
            f"{r['remaining']:,} left   "
            f"({r['funclets_excluded']:,} SEH funclets/thunks excluded)")
    if prev:
        out.append(f"         last sweep {prev.get('date')} ({prev.get('commit')}): "
                   f"{prev.get('verdict')}")
    return out


def last_history() -> dict | None:
    if not HISTORY.exists():
        return None
    try:
        lines = [l for l in HISTORY.read_text(encoding="utf-8").splitlines() if l.strip()]
        return json.loads(lines[-1]) if lines else None
    except (OSError, json.JSONDecodeError, IndexError):
        return None


def freshness_nags() -> list[str]:
    out = []
    nt = REPO / "docs" / "next-targets.md"
    if not nt.exists():
        out.append("next-targets.md missing -> scripts/next_targets.py --write")
    else:
        n = git("rev-list", "--count", "HEAD", "--", "docs/next-targets.md").strip()
        total = git("rev-list", "--count", "HEAD").strip()
        since = git("log", "-1", "--format=%h", "--", "docs/next-targets.md").strip()
        if since:
            behind = git("rev-list", "--count", f"{since}..HEAD").strip()
            if behind.isdigit() and int(behind) > 10:
                out.append(f"next-targets.md is {behind} commits stale "
                           f"-> scripts/next_targets.py --write")
    hook = REPO / ".git" / "hooks" / "pre-commit"
    if hook.exists() and "pre-commit-arc" not in hook.read_text(encoding="utf-8", errors="replace"):
        out.append("pre-commit hook lost the arc delegation "
                   "-> scripts/arc.py hooks --install")
    return out


def cmd_targets(args) -> int:
    script = SCRIPTS / "next_targets.py"
    if not script.exists():
        print("scripts/next_targets.py not present yet", file=sys.stderr)
        return 2
    return subprocess.run([PY, str(script), *args.rest], cwd=REPO).returncode


def cmd_explain(args) -> int:
    rundir = resolve_last_run()
    if rundir is None:
        print("no run recorded under .arc/", file=sys.stderr)
        return 2
    if not args.step:
        print(f"run: {rundir}")
        for p in sorted(rundir.iterdir()):
            print(f"  {p.name}")
        return 0
    shown = False
    for suffix in (".out", ".err"):
        p = rundir / f"{args.step}{suffix}"
        if p.exists():
            print(f"===== {p.name} =====")
            sys.stdout.write(p.read_text(encoding="utf-8", errors="replace"))
            shown = True
    if not shown:
        print(f"no captured output for step '{args.step}' in {rundir}",
              file=sys.stderr)
        return 2
    return 0


def resolve_last_run() -> Path | None:
    last = ARCDIR / "last"
    if last.is_symlink():
        target = ARCDIR / os.readlink(last)
        if target.is_dir():
            return target
    txt = ARCDIR / "last.txt"
    if txt.exists():
        target = ARCDIR / txt.read_text(encoding="utf-8").strip()
        if target.is_dir():
            return target
    if ARCDIR.is_dir():
        runs = sorted((p for p in ARCDIR.iterdir() if p.is_dir()), reverse=True)
        return runs[0] if runs else None
    return None


HOOK_MARKER = "iwd2-re arc gate"
HOOK_BLOCK = f"""
# --- {HOOK_MARKER} (tracked source: scripts/hooks/pre-commit-arc) -------------
# Escape hatch:  ARC_SKIP=1 git commit ...   (skips the gate, KEEPS the graph update)
#                git commit --no-verify      (skips everything, including the graph)
[ -n "$ARC_SKIP" ] && exit 0
_arc="$(git rev-parse --show-toplevel)/scripts/hooks/pre-commit-arc"
# Run through sh rather than exec'ing the file: this repo has core.fileMode
# false, so a lost exec bit would turn the gate into a silent no-op. Missing
# file (fresh clone, bisect into old history) still degrades to a no-op.
[ -f "$_arc" ] && exec sh "$_arc"
exit 0
"""


def cmd_hooks(args) -> int:
    hook = REPO / ".git" / "hooks" / "pre-commit"
    if args.uninstall:
        if not hook.exists():
            print("no pre-commit hook")
            return 0
        text = hook.read_text(encoding="utf-8")
        if HOOK_MARKER not in text:
            print("arc delegation not present")
            return 0
        cut = text.index(f"# --- {HOOK_MARKER}")
        hook.write_text(text[:cut].rstrip() + "\n", encoding="utf-8")
        print("removed arc delegation from .git/hooks/pre-commit")
        return 0

    if not (SCRIPTS / "hooks" / "pre-commit-arc").exists():
        print("scripts/hooks/pre-commit-arc missing", file=sys.stderr)
        return 2
    existing = hook.read_text(encoding="utf-8") if hook.exists() else "#!/bin/sh\n"
    if HOOK_MARKER in existing:
        print("arc delegation already installed")
        return 0
    hook.parent.mkdir(parents=True, exist_ok=True)
    hook.write_text(existing.rstrip() + "\n" + HOOK_BLOCK, encoding="utf-8")
    hook.chmod(0o755)
    print(f"installed arc delegation into {hook.relative_to(REPO)}")
    return 0


# --------------------------------------------------------------------------

def main(argv: list[str]) -> int:
    utf8_stdio()
    ap = argparse.ArgumentParser(
        prog="arc", description=__doc__,
        formatter_class=argparse.RawDescriptionHelpFormatter)
    sub = ap.add_subparsers(dest="cmd", required=True)

    def common(p):
        p.add_argument("--json", action="store_true", help="machine-readable envelope")
        p.add_argument("--strict", action="store_true", help="promote warn to fail")
        p.add_argument("--max-findings", type=int, default=10)
        p.add_argument("--require", action="append",
                       help="capability that must be present (exe/vm/ghidra/exports/cppcheck)")

    c = sub.add_parser("check", help="host-only source lints (the commit gate)")
    c.add_argument("--staged", action="store_true", default=True)
    c.add_argument("--all", action="store_true", help="whole tree instead of the staged diff")
    c.add_argument("--quiet", action="store_true", help="print nothing when clean")
    common(c)
    c.set_defaults(func=cmd_check)

    v = sub.add_parser("verify", help="the full per-arc sequence for one address")
    v.add_argument("address")
    v.add_argument("--slot", type=int, default=3)
    v.add_argument("--hold", type=int, default=90)
    v.add_argument("--save", help="exact MPSave directory name instead of a shifting slot")
    v.add_argument("--new-game", action="store_true", help="native Windows: start a premade new game")
    v.add_argument("--party", default="0", help="native Windows: premade party index/name")
    v.add_argument("--hit", help="symbol the smoke must observe executing")
    v.add_argument("--ui", metavar="SCRIPT",
                   help="AutoUI scenario the smoke replays, so a UI-reachable "
                        "path can actually be driven (see scripts/scenarios/)")
    v.add_argument("--expect", help="regex the debug log must contain after the smoke")
    v.add_argument("--no-build", action="store_true")
    v.add_argument("--no-smoke", action="store_true")
    v.add_argument("--no-parity", action="store_true")
    v.add_argument("--static", action="store_true",
                   help="host-only: no build, no smoke")
    common(v)
    v.set_defaults(func=cmd_verify)

    s = sub.add_parser("sweep", help="whole-codebase auditors")
    s.add_argument("--deep", action="store_true", help="add the whole-tree disasm sweep")
    s.add_argument("--append-history", action="store_true")
    common(s)
    s.set_defaults(func=cmd_sweep)

    st = sub.add_parser("status", help="recovery metrics and freshness nags")
    common(st)
    st.set_defaults(func=cmd_status)

    tg = sub.add_parser("targets", help="proxy to scripts/next_targets.py")
    tg.add_argument("rest", nargs=argparse.REMAINDER)
    tg.set_defaults(func=cmd_targets)

    ex = sub.add_parser("explain", help="captured output of a step from the last run")
    ex.add_argument("step", nargs="?")
    ex.set_defaults(func=cmd_explain)

    hk = sub.add_parser("hooks", help="(re-)install the pre-commit delegation")
    hk.add_argument("--install", action="store_true", default=True)
    hk.add_argument("--uninstall", action="store_true")
    hk.set_defaults(func=cmd_hooks)

    # `targets` forwards its flags verbatim; argparse would eat --top itself.
    if argv and argv[0] == "targets":
        script = SCRIPTS / "next_targets.py"
        if not script.exists():
            print("scripts/next_targets.py not present", file=sys.stderr)
            return 2
        return subprocess.run([PY, str(script), *argv[1:]], cwd=REPO).returncode

    args = ap.parse_args(argv)
    if getattr(args, "static", False):
        args.no_build = args.no_smoke = True
    return args.func(args)


if __name__ == "__main__":
    try:
        raise SystemExit(main(sys.argv[1:]))
    except KeyboardInterrupt:
        raise SystemExit(130)
