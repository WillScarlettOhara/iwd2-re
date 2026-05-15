; AutoHotkey v2 test script for IWD2-re
; Trigger with F1, or run automatically if auto_test.ahk is launched
#Requires AutoHotkey v2.0
#SingleInstance Force
CoordMode "Mouse", "Client"

LOAD_MENU_X := 620
LOAD_MENU_Y := 357
LOAD_SLOT1_X := 720
LOAD_SLOT1_Y := 231

ClickGame(cx, cy) {
    Click cx, cy, "Down"
    Sleep 50
    Click cx, cy, "Up"
}

F1::
{
    if !WinExist("ahk_exe iwd2-re.exe") {
        MsgBox "Game not found!"
        return
    }
    WinActivate "ahk_exe iwd2-re.exe"
    if !WinWaitActive("ahk_exe iwd2-re.exe",, 3) {
        MsgBox "Cannot activate game window"
        return
    }
    Sleep 200
    ClickGame LOAD_MENU_X, LOAD_MENU_Y
    Sleep 1500
    ClickGame LOAD_SLOT1_X, LOAD_SLOT1_Y
}
