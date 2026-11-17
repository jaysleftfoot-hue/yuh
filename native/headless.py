#!/usr/bin/env python3
"""Run the native game on a private software-rendered X11 display."""
import os
from pathlib import Path
import subprocess
import sys
import tempfile
import time
root=Path(__file__).resolve().parents[1]
with tempfile.TemporaryDirectory(prefix='blb-display-') as tmp:
 tmp=Path(tmp)
 config=tmp/'xorg.conf'
 config.write_text('''Section "Device"
 Identifier "Dummy"
 Driver "dummy"
 VideoRam 256000
EndSection
Section "Monitor"
 Identifier "Monitor"
 HorizSync 5.0-1000.0
 VertRefresh 5.0-200.0
 Modeline "1280x800" 83.5 1280 1352 1480 1680 800 803 809 831
EndSection
Section "Screen"
 Identifier "Screen"
 Device "Dummy"
 Monitor "Monitor"
 DefaultDepth 24
 SubSection "Display"
  Depth 24
  Modes "1280x800"
 EndSubSection
EndSection
''')
 display=next(i for i in range(90,150) if not Path(f'/tmp/.X11-unix/X{i}').exists())
 env=dict(os.environ,DISPLAY=f':{display}',LIBGL_ALWAYS_SOFTWARE='1')
 with (tmp/'server.log').open('w') as log:
  server=subprocess.Popen(['/usr/lib/xorg/Xorg',f':{display}','-config',str(config),'-logfile',str(tmp/'xorg.log'),'-noreset','-nolisten','tcp'],stdout=log,stderr=log)
  try:
   for _ in range(100):
    if server.poll() is not None:raise RuntimeError('Xorg startup failed: '+(tmp/'server.log').read_text())
    if subprocess.run(['xdpyinfo'],env=env,stdout=subprocess.DEVNULL,stderr=subprocess.DEVNULL).returncode==0:break
    time.sleep(.1)
   else:raise RuntimeError('Xorg readiness timed out')
   (root/'artifacts').mkdir(exist_ok=True)
   result=subprocess.run([str(root/'dist/native-linux/big-league-bowling')]+sys.argv[1:],cwd=root,env=env,timeout=180)
  finally:
   server.terminate()
   try:server.wait(timeout=5)
   except subprocess.TimeoutExpired:server.kill();server.wait()
 sys.exit(result.returncode)
