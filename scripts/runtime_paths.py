"""Host paths shared by the Linux/VM and native Windows RE workflows."""
from __future__ import annotations

import hashlib
import os
import re
import shutil
import sys
from pathlib import Path

REPO = Path(__file__).resolve().parents[1]
WINDOWS = os.name == "nt"


def venv_tool(name: str, *, windows: bool | None = None, repo: Path = REPO) -> Path:
    configure_environment()
    windows = WINDOWS if windows is None else windows
    return repo / ".venv-reagent" / ("Scripts" if windows else "bin") / (
        name + ".exe" if windows else name)


def python_executable() -> str:
    candidate = venv_tool("python")
    return str(candidate) if candidate.is_file() else sys.executable


def game_directory() -> Path:
    override = os.environ.get("IWD2_GAME_DIR")
    if override:
        return Path(override).expanduser().resolve()
    candidates = [Path(r"C:\Games\Icewind Dale 2"), Path(r"C:\GOG Games\Icewind Dale 2")]
    return next((p for p in candidates if p.is_dir()), candidates[0])


def build_directory() -> Path:
    return Path(os.environ.get("IWD2_BUILD_DIR", str(REPO / "build"))).resolve()


def stage_runtime_companions(exe: Path, game_dir: Path) -> list[Path]:
    """Put the installed DirectDraw companion set beside our Windows executable.

    Windows searches the executable directory before System32 and the game's
    CWD. Keep the installed wrapper and its optional Wine companions together;
    never replace a different DLL already chosen for the build directory.
    An asset-free build is allowed, and an explicit override leaves custom
    runtime setups untouched. Return only the paths newly copied.
    """
    if os.environ.get("IWD2_SKIP_RUNTIME_COMPANIONS") == "1":
        return []
    game_dir = Path(game_dir).resolve()
    if not (game_dir / "ddraw.dll").is_file():
        return []
    destination_dir = Path(exe).resolve().parent
    companions = [(game_dir / name, destination_dir / name)
                  for name in ("ddraw.dll", "libwine.dll", "wined3d.dll")
                  if (game_dir / name).is_file()]

    def verify_existing(source: Path, destination: Path) -> bool:
        if not destination.exists() and not destination.is_symlink():
            return False
        if destination.is_file() and hashlib.sha256(destination.read_bytes()).digest() == \
                hashlib.sha256(source.read_bytes()).digest():
            return True
        raise RuntimeError(
            f"Runtime DLL conflicts with the installed game: {destination} "
            f"(source: {source}). Existing files were preserved. Move the "
            "destination aside to use the game's DLLs, or set "
            "IWD2_SKIP_RUNTIME_COMPANIONS=1 to keep a custom runtime.")

    # Check the whole set first so a known conflict cannot leave a mixed set.
    missing = [(source, destination) for source, destination in companions
               if not verify_existing(source, destination)]
    if not missing:
        return []
    destination_dir.mkdir(parents=True, exist_ok=True)
    copied = []
    for source, destination in missing:
        try:
            output = destination.open("xb")
        except FileExistsError:
            # Another launcher may have staged it since the preflight.
            verify_existing(source, destination)
            continue
        try:
            with output, source.open("rb") as original:
                shutil.copyfileobj(original, output)
        except BaseException:
            destination.unlink(missing_ok=True)
            raise
        copied.append(destination)
    return copied


def runtime_command(*args: str) -> list[str]:
    if WINDOWS:
        return [python_executable(), str(REPO / "scripts" / "native.py"), *args]
    return [str(REPO / "scripts" / "vm.sh"), *args]


def cppcheck_executable() -> str | None:
    override = os.environ.get("IWD2_CPPCHECK")
    if override:
        return override
    found = shutil.which("cppcheck")
    if found:
        return found
    local = REPO / ".tools" / "cppcheck"
    if local.is_dir():
        return next((str(p) for p in local.rglob("cppcheck.exe")), None)
    return None


def configure_environment() -> None:
    """Discover local tools without changing the user's global environment."""
    if not WINDOWS:
        return
    java = os.environ.get("IWD2_JAVA_HOME") or os.environ.get("JAVA_HOME")
    if not java:
        installed = Path(os.environ.get("ProgramFiles", r"C:\Program Files")) / "Java"
        candidates = [p for p in installed.glob("jdk-*") if (p / "bin" / "java.exe").is_file()]
        if candidates:
            java = str(max(candidates, key=lambda p: tuple(int(n) for n in re.findall(r"\d+", p.name))))
    if java:
        os.environ["JAVA_HOME"] = java
    additions = [str(Path(java) / "bin")] if java else []
    cppcheck = cppcheck_executable()
    if cppcheck:
        additions.append(str(Path(cppcheck).parent))
    entries = os.environ.get("PATH", "").split(os.pathsep)
    for directory in reversed(additions):
        if directory not in entries:
            entries.insert(0, directory)
    os.environ["PATH"] = os.pathsep.join(entries)


def utf8_stdio() -> None:
    # Arc and digests print arrows/diagnostics which the Windows ANSI code page
    # cannot encode when captured by a parent process.
    os.environ["PYTHONUTF8"] = "1"
    configure_environment()
    for stream in (sys.stdout, sys.stderr):
        if hasattr(stream, "reconfigure"):
            stream.reconfigure(encoding="utf-8", errors="replace")
