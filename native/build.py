#!/usr/bin/env python3
"""Rebuild the native game using checksum-pinned official raylib SDKs."""
import argparse
import hashlib
import os
from pathlib import Path
import shutil
import subprocess
import tarfile
import zipfile
ROOT=Path(__file__).resolve().parents[1]
p=argparse.ArgumentParser()
p.add_argument('--windows',action='store_true')
p.add_argument('--zig',default=shutil.which('zig'))
p.add_argument('--sdk-cache',type=Path,default=ROOT/'dist/native-sdk')
a=p.parse_args()
a.sdk_cache=a.sdk_cache.resolve()
platform='win64_mingw-w64' if a.windows else 'linux_amd64'
name=f'raylib-5.5_{platform}'
ext='zip' if a.windows else 'tar.gz'
expected='fe8feda1b92fc02826e2d19c631d648fd0563b93b144893fe18140e9fe4fb6fd' if a.windows else '3d95ef03d5b38dfa55c0a16ca122d382134b078f0e5b270b52fe7eae0549c000'
a.sdk_cache.mkdir(parents=True,exist_ok=True)
archive=a.sdk_cache/(name+'.'+ext)
if not archive.exists():
 subprocess.run(['curl','--fail','--location','--retry','2',f'https://github.com/raysan5/raylib/releases/download/5.5/{name}.{ext}','--output',str(archive)],check=True)
if hashlib.sha256(archive.read_bytes()).hexdigest()!=expected:raise SystemExit(f'Checksum mismatch for {archive}; refusing to build')
sdk=a.sdk_cache/name
if not sdk.exists():
 if a.windows:
  with zipfile.ZipFile(archive) as z:
   for item in z.infolist():
    if not (a.sdk_cache/item.filename).resolve().is_relative_to(a.sdk_cache):raise SystemExit('Unsafe archive path')
   z.extractall(a.sdk_cache)
 else:
  with tarfile.open(archive) as t:t.extractall(a.sdk_cache,filter='data')
out=ROOT/'dist'/('native-windows' if a.windows else 'native-linux')
out.mkdir(parents=True,exist_ok=True)
env=os.environ.copy()
env.setdefault('ZIG_GLOBAL_CACHE_DIR',str(a.sdk_cache/'zig-global'))
env.setdefault('ZIG_LOCAL_CACHE_DIR',str(a.sdk_cache/'zig-local'))
common=['-std=c++17','-O2','-Wall','-Wextra','-I'+str(ROOT/'native/src')]
if a.windows:
 if not a.zig:raise SystemExit('--zig must name a Zig 0.14.1 executable')
 compiler=[a.zig,'c++','-target','x86_64-windows-gnu']
 libs=['-lopengl32','-lgdi32','-lwinmm','-static','-s','-Wl,--subsystem,windows']
 binary=out/'Big League Bowling.exe'
else:
 compiler=['g++'];libs=['-l:libGL.so.1','-l:libX11.so.6','-lm','-lpthread','-ldl','-lrt'];binary=out/'big-league-bowling'
 for name in ['rules','campaign']:
  subprocess.run(compiler+common+[str(ROOT/f'native/tests/{name}.cpp'),'-o',str(out/(name+'-tests'))],check=True)
  subprocess.run([str(out/(name+'-tests'))],check=True)
subprocess.run(compiler+common+['-isystem',str(sdk/'include'),str(ROOT/'native/src/main.cpp'),str(sdk/'lib/libraylib.a')]+libs+['-o',str(binary)],check=True,env=env)
shutil.copytree(ROOT/'native/assets',out/'assets',dirs_exist_ok=True)
shutil.copy2(sdk/'LICENSE',out/'RAYLIB-LICENSE.txt')
shutil.copy2(ROOT/'native/README.md',out/'README.txt')
shutil.copy2(ROOT/'native/ART_DIRECTION.md',out/'ART_DIRECTION.md')
if a.windows:
 release=ROOT/'dist/Big-League-Bowling-Native-Windows-x64.zip'
 with zipfile.ZipFile(release,'w',zipfile.ZIP_DEFLATED) as z:
  for f in sorted(out.rglob('*')):
   if f.is_file() and f.suffix!='.pdb':z.write(f,f.relative_to(out))
 print(release)
print(binary)
