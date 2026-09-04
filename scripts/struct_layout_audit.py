#!/usr/bin/env python3
"""Struct-layout drift auditor for IWD2.exe binary-mirror classes.

WHY THIS EXISTS
    A binary-mirror class (members carry /* 0xNNN */ offset comments) only matches
    IWD2.exe if the COMPILER places every member at the commented offset. The IE
    engine packs its structs to 2 bytes; a class that forgets `#pragma pack(2)`
    lets the compiler 4-align a sub-4-aligned LONG/WORD tail, drifting it a couple
    bytes. The code still compiles, parity stays GREEN (parity checks code, not
    layout), and the only symptom is a runtime/visual bug -- e.g. the Cloudkill
    detonation over-density: m_visual2MaxSpawn drifted +0x30 vs the packed +0x2E,
    so IcewindCProjectileSpellHit::OnArrival's reinterpret_cast read garbage, the ring spawn
    ran uncapped (36 particles vs 13).

WHAT IT DOES
    Compares the MSVC-COMPILED layout (cl /d1reportSingleClassLayout<Class>, the
    real offsets) against the header's /* 0xNNN */ comments. The comparison is on
    DELTAS between members, not absolute offsets, so it is invariant to the known
    CGameObject base-size discrepancy (compiled 0x78 vs binary 0x6E) that shifts
    every leaf -- only genuine *spacing* drift (the packing bug) trips it.

    offsetof()-based static_asserts can't do this: offsetof on a polymorphic
    (non-standard-layout) class like CProjectile is rejected by MSVC (C2618).
    The compiler layout dump has no such limit and covers every member.

USAGE
    python scripts/struct_layout_audit.py CProjectile
    python scripts/struct_layout_audit.py CProjectile --header src/CProjectile.h \
                                                       --source src/CProjectile.cpp
    Each break is auto-classified, keyed on the MOVED member (not the preceding
    type, which used to mask CVidCell-preceded sub-4 LONGs):
      ACTIONABLE   -- moved member is sub-4-aligned (offset % 4 == 2) with a +2 pad
      benign       -- preceding member is a compiler-divergent type (CVidCell/STL)
      cumulative   -- a 4-aligned member an upstream pack site pushed off-boundary
      review       -- unknown embedded-class size, or a non-positive drift artifact
      out-of-order -- member declared below the prior member's offset
    Exit 1 = >=1 actionable or out-of-order break, 0 = clean / benign-only,
    2 = harness/parse error.

    Windows: compile only the selected TU using the configured VS2019/v142
    Debug Win32 project, into a separate temporary intermediate directory.
    No link, source timestamp change, or game stop is needed. Configure/build
    once with native.py first. --build-dir / IWD2_BUILD_DIR selects the project.
    Linux: keep the VM route; sync header/source, then force a TU recompile.
"""
import argparse
import os
import re
import subprocess
import sys
import tempfile
from pathlib import Path

from runtime_paths import WINDOWS, build_directory, utf8_stdio

VM = "win11vm"
VM_REPO = "C:/iwd2-re"
HOST_REPO = Path(__file__).resolve().parent.parent


class AuditError(RuntimeError):
    """The compiler did not produce usable layout evidence."""

# Header member with an offset comment, e.g.  "/* 053C */ LONG m_visual2MaxSpawn;"
# or "/* 04C4 */ std::map<LONG, int> m_miniA;" or "BYTE _pad[8];".
_HDR_MEMBER = re.compile(
    r"/\*\s*(?:0x)?([0-9A-Fa-f]+)\s*\*/\s*(.*?)\b([A-Za-z_]\w*)\s*(?:\[[^\]]*\])?\s*;")

# Classify a spacing break by the PRECEDING member's type. The gap INTO a member
# grows either because the preceding member's compiled size differs (benign when
# that type is compiler-divergent) or because alignment padding was inserted
# before the *following* member -- it wants >2-byte alignment that #pragma pack(2)
# would have removed (the actionable packing bug, e.g. the Cloudkill visual slot).
_DIVERGENT = re.compile(
    r"\bCVidCell\b|\bstd\s*::|\bCArray\s*<|\bCTypedPtr\w*\s*<|\bCMap\w*\s*<|"
    r"\bCList\b|\bCPtrList\b")
_STABLE = re.compile(
    r"\b(?:BYTE|WORD|DWORD|ULONG|LONG|UINT|INT|int|unsigned|USHORT|SHORT|"
    r"BOOLEAN|BOOL|bool|CHAR|char|float|double|COLORREF)\b|\*\s*$")


def classify(prev_type: str, offset: int, drift: int) -> str:
    """Bucket a spacing break, keying FIRST on the moved member's own binary
    sub-alignment so a divergent preceding type can no longer mask a real pack
    site (the CVidCell-preceded sub-4 LONGs the prev-type heuristic missed):
    actionable | benign | cumulative | review."""
    if offset % 4 != 0 and drift == 2:
        # The binary sits a 4-align-wanting member at a 2-aligned (offset % 4 == 2)
        # slot and the compiler padded it +2 to a 4-boundary -- the pack(2) bug,
        # whatever precedes it (this is what a divergent prev used to hide).
        return "actionable"
    if _DIVERGENT.search(prev_type):
        # A member the binary already 4-aligns, drifted by the preceding member's
        # compiled size: a known CVidCell / STL / MFC discrepancy, not a pack bug.
        return "benign"
    if drift <= 0:
        # A pack pad is strictly positive; a non-positive drift is a
        # declaration-order-vs-offset artifact (successors carry lower offsets).
        return "review"
    if _STABLE.search(prev_type):
        # Upstream drift bled past a fixed-size member into a 4-aligned one; pack(2)
        # would not move it -- it resolves once the upstream pack site is fixed.
        return "cumulative"
    return "review"          # unknown embedded-class size -- verify by hand
# MSVC layout dump member line, e.g.  " 1398\t| m_visual2MaxSpawn"  (single pipe =
# a direct member; "| |" = an inherited base member, skipped).
_DUMP_MEMBER = re.compile(r"^\s*(\d+)\s*\|\s+(?!\||\+)(.*\b)([A-Za-z_]\w*)\s*$")


def parse_header(header: Path, cls: str):
    """Ordered [(member, comment_offset, type)] for the direct members of `cls`."""
    text = header.read_text(encoding="utf-8", errors="replace").splitlines()
    # Find the class body and walk it at brace-depth 1 (skip nested structs).
    # The class *definition* (name then ':' base-list, a '/*..*/' tag, or '{') --
    # not a forward declaration ("class X;").
    start = next((i for i, ln in enumerate(text)
                  if re.search(rf"\bclass\s+{re.escape(cls)}\b\s*(:|/\*|\{{)", ln)), None)
    if start is None:
        raise AuditError(f"class {cls} not found in {header}")
    members, depth, seen = [], 0, False
    for ln in text[start:]:
        opens, closes = ln.count("{"), ln.count("}")
        # A member counts only at the class's own scope (depth 1, outside nested {}).
        at_class_scope = seen and depth == 1
        if at_class_scope:
            m = _HDR_MEMBER.search(ln)
            if m:
                members.append((m.group(3), int(m.group(1), 16), m.group(2).strip()))
        depth += opens - closes
        if opens:
            seen = True
        if seen and depth <= 0:
            break
    return members


def native_msbuild(build_dir: Path) -> Path:
    """Use the same VS2019 instance and Win32 project as the normal build."""
    cache_path = build_dir / "CMakeCache.txt"
    if not cache_path.is_file() or not (build_dir / "iwd2-re.vcxproj").is_file():
        raise AuditError("configure the native build first: python scripts/native.py build")
    cache = cache_path.read_text(encoding="utf-8", errors="replace")
    if "CMAKE_GENERATOR:INTERNAL=Visual Studio 16 2019" not in cache or \
            "CMAKE_GENERATOR_PLATFORM:INTERNAL=Win32" not in cache:
        raise AuditError("layout audit requires a Visual Studio 16 2019 / Win32 build directory")
    instance = re.search(r"^CMAKE_GENERATOR_INSTANCE:INTERNAL=(.+)$", cache, re.M)
    if instance and instance[1].strip():
        installation = instance[1].strip()
    else:
        vswhere = Path(os.environ.get("ProgramFiles(x86)", r"C:\Program Files (x86)")) / \
            "Microsoft Visual Studio" / "Installer" / "vswhere.exe"
        found = subprocess.run(
            [str(vswhere), "-latest", "-products", "*", "-version", "[16.0,17.0)",
             "-requires", "Microsoft.VisualStudio.Component.VC.Tools.x86.x64",
             "-property", "installationPath"],
            check=True, capture_output=True, text=True, encoding="utf-8", timeout=30)
        installation = found.stdout.strip()
    msbuild = Path(installation) / "MSBuild" / "Current" / "Bin" / "MSBuild.exe"
    if not installation or not msbuild.is_file():
        raise AuditError("VS2019 MSBuild not found; install the v142 C++ and MFC components")
    return msbuild


def native_compile_command(msbuild: Path, project: Path, source: Path, scratch: Path) -> list[str]:
    """Compile one TU with the project's real flags, without touching its normal outputs."""
    return [str(msbuild), str(project), "/nologo", "/v:normal", "/t:ClCompile",
            "/p:Configuration=Debug", "/p:Platform=Win32", "/p:BuildProjectReferences=false",
            f"/p:SelectedFiles={source}", f"/p:IntDir={scratch}{os.sep}",
            "/p:TrackFileAccess=false", "/p:UseMultiToolTask=false"]


def run_native_layout_dump(cls: str, source_rel: str, build_dir: Path) -> str:
    msbuild = native_msbuild(build_dir)
    source = (HOST_REPO / source_rel).resolve()
    if not source.is_file():
        raise AuditError(f"source TU missing: {source}")
    logs = HOST_REPO / ".arc" / "layout"
    logs.mkdir(parents=True, exist_ok=True)
    log = logs / f"{cls.replace('::', '_')}.log"
    env = os.environ.copy()
    env["CL"] = f"{env.get('CL', '')} /d1reportSingleClassLayout{cls}".strip()
    with tempfile.TemporaryDirectory(prefix="compile-", dir=logs) as scratch:
        command = native_compile_command(msbuild, build_dir / "iwd2-re.vcxproj",
                                         source, Path(scratch))
        result = subprocess.run(command, cwd=build_dir, env=env, capture_output=True,
                                encoding="utf-8", errors="replace", timeout=600)
        output = result.stdout + result.stderr
        log.write_text(output, encoding="utf-8")
    if result.returncode:
        raise AuditError(f"native MSVC compile failed ({result.returncode}); see {log}")
    print(f"[audit] native compiler output: {log}")
    return output


def run_layout_dump(cls: str, source_rel: str, build_dir: Path | None = None) -> str:
    """Return a real compiler layout using the current host's runtime route."""
    if WINDOWS:
        return run_native_layout_dump(cls, source_rel, (build_dir or build_directory()).resolve())
    # Linux/VM: force-recompile the synced TU with the layout flag.
    src_vm = f"{VM_REPO}/{source_rel}".replace("/", "\\")
    build_vm = f"{VM_REPO}/scripts/vm_build.cmd".replace("/", "\\")
    # Sent verbatim to PowerShell on the VM (subprocess list -> no local shell, so
    # $env is PowerShell's and needs no escaping): bump the TU's mtime so cmake
    # recompiles it, set CL so cl.exe emits the layout, then build.
    ps = (
        f"(Get-Item {src_vm}).LastWriteTime = Get-Date; "
        f"$env:CL='/d1reportSingleClassLayout{cls}'; "
        f"cmd /c {build_vm}"
    )
    out = subprocess.run(["ssh", VM, ps],
                         capture_output=True, text=True, timeout=600)
    if out.returncode:
        raise AuditError(f"VM layout compile failed ({out.returncode}): "
                         + (out.stderr.strip() or out.stdout.strip())[-1000:])
    return out.stdout + out.stderr


def parse_dump(dump: str, cls: str):
    """member -> compiled_offset, from the base `class <cls>` block (single pipe)."""
    lines = dump.splitlines()
    start = next((i for i, ln in enumerate(lines)
                  if re.match(rf"^\s*class\s+{re.escape(cls)}\s+size\(", ln)), None)
    if start is None:
        raise AuditError(f"no '/d1reportSingleClassLayout{cls}' block in compiler output "
                         "(check the selected TU and compiler log)")
    offsets = {}
    for ln in lines[start + 1:]:
        if re.match(r"^\s*class\s", ln):        # next class block -> end
            break
        m = _DUMP_MEMBER.match(ln)
        if m:
            offsets.setdefault(m.group(3), int(m.group(1)))
    return offsets


def main():
    utf8_stdio()
    ap = argparse.ArgumentParser(description="Audit a binary-mirror class for layout drift.")
    ap.add_argument("cls", help="class name, e.g. CProjectile")
    ap.add_argument("--header", help="header file (default: src/<cls>.h)")
    ap.add_argument("--source", help="TU that defines the class (default: src/<cls>.cpp)")
    ap.add_argument("--build-dir", type=Path, default=build_directory(),
                    help="native CMake build directory (or IWD2_BUILD_DIR)")
    args = ap.parse_args()
    if not re.fullmatch(r"[A-Za-z_]\w*(?:::[A-Za-z_]\w*)*", args.cls):
        raise AuditError("class must be a C++ identifier, optionally namespace-qualified")

    header = Path(args.header) if args.header else HOST_REPO / "src" / f"{args.cls}.h"
    source_rel = args.source or f"src/{args.cls}.cpp"
    if not header.is_absolute():
        header = HOST_REPO / header

    hdr = parse_header(header, args.cls)
    if not hdr:
        raise AuditError(f"no /* 0xNNN */ member comments for {args.cls} in {header}")

    # Sync the files under audit so the compiled layout matches the comments.
    if not WINDOWS:
        for rel in {f"src/{header.name}", source_rel}:
            subprocess.run(["scp", "-q", str(HOST_REPO / rel), f"{VM}:{VM_REPO}/{rel}"],
                           check=True, timeout=120)

    dump = run_layout_dump(args.cls, source_rel, args.build_dir)
    compiled = parse_dump(dump, args.cls)

    common = [(m, off, typ) for m, off, typ in hdr if m in compiled]
    if len(common) < 2:
        raise AuditError(f"only {len(common)} member(s) matched between header and dump "
                         "-- cannot diff spacing")

    # Adjacent-member spacing: a uniform base/STL size shift (e.g. our CGameObject
    # 0x78 vs the binary's 0x6E, or a wider STL node) cancels out, so only a genuine
    # spacing change trips. Each break is then classified by the preceding member's
    # type -- benign (compiler-divergent size) vs actionable (a sub-aligned tail that
    # wants pack(2)) -- and a member declared below the prior offset is flagged as
    # out-of-order (the compiler lays members in declaration order; the spacing of
    # its neighbour pair is meaningless, so that pair is skipped).
    actionable, benign, cumulative, review, out_of_order = [], [], [], [], []
    prev_oo = False
    for (pname, pcmt, ptyp), (name, cmt, _t) in zip(common, common[1:]):
        if cmt <= pcmt:                       # declared at/below the prior offset
            out_of_order.append((pname, name, pcmt, cmt))
            prev_oo = True
            continue
        if prev_oo:                           # compiled gap polluted by the oo member
            prev_oo = False
            continue
        cmt_gap, bin_gap = cmt - pcmt, compiled[name] - compiled[pname]
        if cmt_gap == bin_gap:
            continue
        row = (pname, name, cmt_gap, bin_gap, ptyp)
        {"benign": benign, "actionable": actionable, "cumulative": cumulative,
         "review": review}[classify(ptyp, cmt, bin_gap - cmt_gap)].append(row)

    def _show(rows):
        for prev, name, cg, bg, ptyp in rows:
            print(f"   {prev} -> {name}: comment gap 0x{cg:X}, compiled gap 0x{bg:X} "
                  f"(drift {bg - cg:+d} B) [prev {ptyp or '?'}]")

    print(f"[audit] {args.cls}: {len(common)} members checked vs /* 0xNNN */ comments")
    if actionable:
        print(f"[audit] {len(actionable)} ACTIONABLE -- a sub-aligned tail drifted for "
              f"want of #pragma pack(2). Real bug if a reinterpret_cast or a "
              f"hardcoded/binary offset reads the moved member; named/by-pointer access "
              f"only = latent layout mismatch. Verify the use site:")
        _show(actionable)
    if out_of_order:
        print(f"[audit] {len(out_of_order)} OUT-OF-ORDER -- member declared below the "
              f"prior member's offset. The compiler lays members in declaration order, "
              f"so the layout cannot match the binary until they are reordered:")
        for prev, name, pcmt, cmt in out_of_order:
            print(f"   {prev} (0x{pcmt:X}) -> {name} (0x{cmt:X}): offset goes backwards")
    if benign:
        print(f"[audit] {len(benign)} benign -- preceding member is a compiler-divergent "
              f"type (CVidCell / STL / MFC container), a known whole-codebase size "
              f"discrepancy, not a packing bug:")
        _show(benign)
    if cumulative:
        print(f"[audit] {len(cumulative)} cumulative -- the moved member is 4-aligned in "
              f"the binary, so pack(2) won't move it; its compiled pad is an upstream "
              f"benign break bleeding through an alignment boundary, not a fix site:")
        _show(cumulative)
    if review:
        print(f"[audit] {len(review)} review -- preceding member's compiled size is "
              f"unknown to the classifier (an embedded class); verify by hand:")
        _show(review)
    if not (actionable or out_of_order or benign or cumulative or review):
        print("[audit] PASS -- compiled member spacing matches the comments.")
        return 0
    if actionable or out_of_order:
        print(f"[audit] RESULT: {len(actionable)} actionable + {len(out_of_order)} "
              f"out-of-order -> FAIL")
        return 1
    print("[audit] RESULT: only benign/review breaks -> PASS")
    return 0


if __name__ == "__main__":
    try:
        sys.exit(main())
    except (AuditError, OSError, subprocess.SubprocessError) as exc:
        print(f"[audit] ERROR: {exc}", file=sys.stderr)
        sys.exit(2)
