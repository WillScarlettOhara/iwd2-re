Add-Type @'
using System;
using System.Runtime.InteropServices;
public static class U32 {
  [DllImport("user32.dll")]
  public static extern bool SetCursorPos(int X, int Y);
  [DllImport("user32.dll")]
  public static extern void mouse_event(uint dwFlags, uint dx, uint dy, uint dwData, UIntPtr dwExtraInfo);
}
'@

$p = Start-Process 'C:\iwd2-re\build\Debug\iwd2-re.exe' -WorkingDirectory 'C:\GOG Games\Icewind Dale 2' -PassThru
Start-Sleep -Seconds 5
# Bouton "New Game" approximatif (fenêtre ~568,235 + offset menu ~560,599)
[U32]::SetCursorPos(1128, 834) | Out-Null
Start-Sleep -Milliseconds 400
[U32]::mouse_event(0x0002, 0, 0, 0, [UIntPtr]::Zero)
Start-Sleep -Milliseconds 600
[U32]::mouse_event(0x0004, 0, 0, 0, [UIntPtr]::Zero)
Start-Sleep -Seconds 3
if (-not $p.HasExited) {
  Stop-Process -Id $p.Id -Force
}
