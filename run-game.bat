@echo off
setlocal
set "IWD2_PYTHON=%~dp0.venv-reagent\Scripts\python.exe"
if not exist "%IWD2_PYTHON%" (
    echo [iwd2-re] Analysis environment not found. Follow BUILD_WINDOWS.md first.
    pause
    exit /b 1
)
"%IWD2_PYTHON%" "%~dp0scripts\native.py" menu
if errorlevel 1 (
    pause
    exit /b 1
)
endlocal
