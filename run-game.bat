@echo off
setlocal

if not defined IWD2_GAME_DIR set "IWD2_GAME_DIR=C:\GOG Games\Icewind Dale 2"
set "IWD2_RE_EXE=%~dp0build\Debug\iwd2-re.exe"

if not exist "%IWD2_RE_EXE%" (
    echo [iwd2-re] executable not found:
    echo   %IWD2_RE_EXE%
    echo Build first with:
    echo   cmake --build build --config Debug
    pause
    exit /b 1
)

if not exist "%IWD2_GAME_DIR%\Chitin.key" (
    echo [iwd2-re] Chitin.key not found in game directory:
    echo   %IWD2_GAME_DIR%
    echo.
    echo Set IWD2_GAME_DIR to your install path, for example:
    echo   set IWD2_GAME_DIR=D:\Games\Icewind Dale 2
    pause
    exit /b 1
)

echo [iwd2-re] Launching from:
echo   %IWD2_GAME_DIR%
start "iwd2-re" /D "%IWD2_GAME_DIR%" "%IWD2_RE_EXE%"

endlocal
