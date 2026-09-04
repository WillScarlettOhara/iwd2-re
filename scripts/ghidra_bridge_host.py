"""Configuration and JDK-aware bridge entry point for native Windows."""
import subprocess
import sys

from runtime_paths import REPO, utf8_stdio, venv_tool


def main() -> int:
    utf8_stdio()
    return subprocess.run([str(venv_tool("ghidra-bridge")), "--config",
                           str(REPO / "ghidra-bridge.host.yaml"), *sys.argv[1:]],
                          cwd=REPO).returncode


if __name__ == "__main__":
    raise SystemExit(main())
