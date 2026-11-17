# Big League Bowling — native C++ arcade

C++17 + raylib 5.5 desktop prototype. The native work lost in the cloud refresh was reconstructed from the development conversation on November 17, 2026. This is a tested reconstruction, not a byte-for-byte recovery of the missing filesystem. The earlier browser/Electron game remains in the repository.

## Playable systems

- Traditional ball bowling against a simulated AI opponent: position, aim, timed release, power and curve.
- Mascot Ride: an original ray mascot on a baseball sled; steering after launch and one boost per shot.
- Heavy Ball, Magnet and Bumpers; two charges of each per player per match.
- Ten-frame bowling with strikes, spares and tenth-frame bonus balls. Players alternate after each completed frame, with independent racks.
- Exactly 64 named unlocks: 30 MLB team color/initial designs, 30 playful team-inspired pattern designs, and four Rays specials (Sunburst, Devil Ray, Touch Tank, Tropicana Finale). The first ball is unlocked; each 80 XP unlocks the next, all 64 at 5,040 XP.
- Completed matches award 120 + score XP, plus 150 for a win. Abandoned matches award nothing.
- Ten stadium-themed layouts open by winning at the previous stop: Wrigley, Fenway, Oracle, PNC, Coors, Kauffman, Chase, T-Mobile, Yankee, and **Tropicana Field last**.
- Local profile with XP, wins, stadium unlocks and best score. Saved under APPDATA, XDG_DATA_HOME or HOME as big-league-bowling-profile.txt, with .bak recovery. Failed persistence is reported on screen.

## Controls

A/D position; arrows aim; Q/E curve; sliders adjust all shot values. Hold Space to time power and release to bowl, or press LET IT RIP. In Mascot Ride, arrows/A/D steer after launch and Space boosts once. Keys 1/2/3 select a power before launch. Escape pauses, M toggles sound, F11 toggles fullscreen. The clubhouse selects mode, stadium and ball. A rematch is available after scoring.

## Graphics and limits

The latest scene includes neighboring lanes, pinsetters, stadium signs, brick concourse walls, balconies, crowds, trusses, light panels and tables. The ray has curved wing meshes, a stitched sled, jersey, animated head/eyes and segmented tail. Lighting includes warm/cool illumination, view-dependent specular highlights, fog, wood grain, seams and approximate contact shadows. HUD has metal frames and illustrated powers.

This remains a procedural prototype, well short of the requested 2005–2010 reference fidelity. Team designs use colors and initials rather than official logos. Stadiums are themed interpretations sharing a lane layout, not architectural replicas. No obstacle courses, collectible routes, voice acting, advanced rigid-body physics or reference-matched video controls are implemented. Audio is synthesized; the cloud machine lacks a playback device. Read ART_DIRECTION.md before continuing graphics/gameplay work.

## Linux / cloud build

Requires Linux x64, Python 3.12+, curl, g++, libGL.so.1 and libX11.so.6. A valid X11 display and OpenGL 3.3 are needed to play.

```
python3 native/build.py
./dist/native-linux/big-league-bowling
```

The build script checks official raylib SDK archive SHA-256 hashes, preserves TLS verification, builds and runs rules/campaign tests, and copies assets/licenses beside the game. Downloads stay in ignored dist/native-sdk. `--sdk-cache /workspace/tools` reuses the cloud cache.

On the managed cloud machine, Xorg and its dummy driver can run native graphical smoke checks:

```
python3 native/headless.py --smoke
python3 native/headless.py --smoke --mascot
python3 native/headless.py --play --mascot --arena 9 --screenshot /workspace/yuh/artifacts/tropicana.png
```

Headless creates a private display, runs a real native window, then cleans up its server. A smoke test completes both competitors' games, checks roll count and exits nonzero on failure. Smoke mode does not save progression. --arena is a nonpersistent developer preview. Screenshot-only mode captures after 90 frames and exits. `artifacts/` is created by the headless helper.

## Windows package

Install Zig 0.14.1 and run:

```
python3 native/build.py --windows --zig /absolute/path/to/zig
```

Output: dist/Big-League-Bowling-Native-Windows-x64.zip. Extract it completely and run Big League Bowling.exe. Keep assets/ beside the EXE. No browser, Node or Python is needed to play. Requires Windows 10/11 x64 with OpenGL 3.3-capable graphics. The EXE is unsigned. It has been cross-built on Linux; execution on Windows remains unverified.

Alternatively, install Visual Studio 2022 C++ tools, Git and CMake on Windows:

```
cmake -S native -B dist/native-build
cmake --build dist/native-build --config Release --parallel 2
ctest --test-dir dist/native-build -C Release --output-on-failure
```

CMake fetches raylib 5.5 from its upstream tag. The MSVC route is documented but not verified in this Linux environment. For the checked official Linux SDK, pass `-DRAYLIB_ROOT=/absolute/path/raylib-5.5_linux_amd64` to CMake.

## Preservation

Source, shader, font/license, procedural assets, tests, build instructions, art direction and verification screenshots belong in Git. Generated binaries and dependency caches stay out of source history and are rebuilt with the scripts. Follow ../AGENTS.md: commit and push completed patches, verify remote HEAD, and never rely solely on the cloud workspace. A draft environment save is not a source backup.
