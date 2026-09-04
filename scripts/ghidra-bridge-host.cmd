@echo off
setlocal
set "REPO=%~dp0.."
set "PYTHONUTF8=1"
"%REPO%\.venv-reagent\Scripts\python.exe" "%~dp0ghidra_bridge_host.py" %*
exit /b %errorlevel%
