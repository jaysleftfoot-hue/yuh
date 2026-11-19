#!/usr/bin/env python3
"""Native X11 input regression: python3 native/tests/ui_flow.py phases (or keyboard/setup/locker/locked/pause/help/scores/mascot/mascot-pull/mascot-shot/mascot-straight/curve/power). Build Linux first; requires Xorg dummy, libX11 and libXtst. Uses an isolated display and non-saving screenshot mode."""
import ctypes as c,subprocess,time,sys,os
from pathlib import Path
x=c.CDLL('libX11.so.6');t=c.CDLL('libXtst.so.6')
x.XOpenDisplay.argtypes=[c.c_char_p];x.XOpenDisplay.restype=c.c_void_p
x.XFlush.argtypes=[c.c_void_p]
x.XKeysymToKeycode.argtypes=[c.c_void_p,c.c_ulong];x.XKeysymToKeycode.restype=c.c_uint
t.XTestFakeKeyEvent.argtypes=[c.c_void_p,c.c_uint,c.c_int,c.c_ulong]
t.XTestFakeMotionEvent.argtypes=[c.c_void_p,c.c_int,c.c_int,c.c_int,c.c_ulong]
t.XTestFakeButtonEvent.argtypes=[c.c_void_p,c.c_uint,c.c_int,c.c_ulong]
os.chdir(Path(__file__).resolve().parents[2])
scenario=sys.argv[1]
assert scenario in ['phases','keyboard','setup','locker','locked','pause','help','scores','mascot','mascot-pull','mascot-shot','mascot-straight','curve','power','equipment']
old=set(Path('/tmp/.X11-unix').glob('X*'));log=Path('/tmp/arcade-'+scenario+'.log')
with log.open('w') as f:
 p=subprocess.Popen(['python3','native/headless.py','--capture-frame','3600','--screenshot','artifacts/arcade-'+scenario+'.png'],stdout=f,stderr=f)
 for _ in range(150):
  new=set(Path('/tmp/.X11-unix').glob('X*'))-old
  if new and 'UI preview:' in log.read_text():break
  time.sleep(.1)
 d=x.XOpenDisplay((':'+next(iter(new)).name[1:]).encode());assert d
 def move(px,py):t.XTestFakeMotionEvent(d,-1,px,py,0);x.XFlush(d);time.sleep(.25)
 def click(px,py):
  move(px,py)
  t.XTestFakeButtonEvent(d,1,1,0);x.XFlush(d);time.sleep(.25)
  t.XTestFakeButtonEvent(d,1,0,0);x.XFlush(d);time.sleep(.6)
 def key(sym):
  k=x.XKeysymToKeycode(d,sym)
  t.XTestFakeKeyEvent(d,k,1,0);x.XFlush(d);time.sleep(.15)
  t.XTestFakeKeyEvent(d,k,0,0);x.XFlush(d);time.sleep(.4)
 if scenario=='locker':click(235,739);click(767,628)
 elif scenario=='help':click(810,739)
 elif scenario=='scores':click(420,739)
 else:
  click(810 if scenario.startswith('mascot') else 440,675)
  if scenario=='setup':
   for _ in range(3):key(0xff53)
  if scenario not in ['setup','locked']:
   if scenario=='mascot-straight':click(900,606)
   if scenario=='keyboard':key(0xff0d)
   else:click(510,682)
   if scenario in ['mascot-pull','mascot-shot','mascot-straight']:
    click(415,542)
    assert 'Mascot launch:' not in log.read_text(), 'A click without drawing the sling launched'
    move(415,542)
    t.XTestFakeButtonEvent(d,1,1,0);x.XFlush(d);time.sleep(.25)
    move(615,682)
    if scenario!='mascot-pull':
     t.XTestFakeButtonEvent(d,1,0,0);x.XFlush(d);time.sleep(.5)
     line=next(l for l in log.read_text().splitlines() if l.startswith('Mascot launch:'))
     values=dict(part.split('=') for part in line.split()[2:])
     assert 2.68<float(values['position'])<2.77 and float(values['aim'])==0, line
     assert float(values['power'])==1 and int(values['pickups'])==(scenario!='mascot-straight'), line
   if scenario=='equipment':click(78,287)
   if scenario=='pause':key(0xff1b)
   if scenario=='keyboard':
    assert 'Shot phase:' not in log.read_text(), 'Setup key release advanced shot'
    key(0x20);key(0x20);key(0xff1b);key(0xff0d)
    assert log.read_text().count('Shot phase:')==2, 'Resume key release launched'
    key(0xff08);key(0x20);key(0x20)
   if scenario=='curve':click(600,600);move(780,600)
   if scenario in ['power','phases']:
    click(600,600);click(780,600)
    if scenario=='phases':click(820,600)
  elif scenario=='locked':click(825,240);click(510,682)
 time.sleep(.6)
 key(0xffc9)
 assert p.wait(timeout=150)==0,log.read_text()[-2000:]
 extra=Path('screenshot000.png')
 if extra.exists():extra.replace('artifacts/raylib-last-capture.png')
 print('\n'.join(l for l in log.read_text().splitlines() if l.startswith(('UI preview','Shot phase'))))
 if scenario=='phases':
  lines=[l for l in log.read_text().splitlines() if l.startswith('Shot phase')]
  assert len(lines)==3 and 'phase: 1 state=0' in lines[0] and 'phase: 2 state=0' in lines[1] and 'state=1' in lines[2],lines
 if scenario=='locked':assert 'screen=1' not in log.read_text()
