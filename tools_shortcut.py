import os
import subprocess

desktop = os.path.join(os.environ['USERPROFILE'], 'Desktop')
lnk_path = os.path.join(desktop, 'GhostHUD.lnk')
exe_path = r'C:\Users\iuryc\ghost-hud\GhostHUD.exe'
ico_path = r'C:\Users\iuryc\ghost-hud\ghosthud.ico'
work_dir = r'C:\Users\iuryc\ghost-hud'

ps_script = f"""
$ws = New-Object -ComObject WScript.Shell
$sc = $ws.CreateShortcut('{lnk_path}')
$sc.TargetPath = '{exe_path}'
$sc.WorkingDirectory = '{work_dir}'
$sc.IconLocation = '{ico_path}'
$sc.Save()
"""

subprocess.run(["powershell", "-NoProfile", "-Command", ps_script], check=True)
print("Desktop shortcut created with custom icon:", lnk_path)
