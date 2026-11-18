# Big League Bowling — native C++ arcade

C++17 + raylib 5.5 desktop prototype. The native work lost in the cloud refresh was reconstructed from the development conversation on November 17, 2026. This is a tested reconstruction, not a byte-for-byte recovery of the missing filesystem. The earlier browser/Electron game remains in the repository.

## Playable systems

- Stadium bowling against an AI rival with separate spot, curve and power phases.
- Rookie, Pro and All-Star rivals in both modes. Click the AI label in Ballpark Showdown to cycle the difficulty. Pro is the default, and the selection carries into rematches.
- The pin triangle and standing count follow the active rack in real time, including spare attempts and changes of player.
- Mascot Ride: an original ray mascot on an inflatable tube; steering after launch and one boost per shot.
- Heavy Ball, Magnet and Bumpers; two charges of each per player per match.
- Ten-frame bowling with strikes, spares and tenth-frame bonus balls. Players alternate after each completed frame, with independent racks.
- Exactly 64 named unlocks: 30 MLB logo balls, 30 individually illustrated club designs, and four Rays specials (Sunburst, Devil Ray, Touch Tank, Tropicana Finale). Use left/right arrows in the locker to inspect the reverse artwork. The first ball is unlocked; each 80 XP unlocks the next, all 64 at 5,040 XP.
- Completed matches award 120 + score XP, plus 150 for a win. Abandoned matches award nothing.
- Ten stadium-themed layouts open by winning at the previous stop: Wrigley, Fenway, Oracle, PNC, Coors, Kauffman, Chase, T-Mobile, Yankee, and **Tropicana Field last**.
- Local profile with XP, wins, stadium unlocks and best score. Saved under APPDATA, XDG_DATA_HOME or HOME as big-league-bowling-profile.txt, with .bak recovery. Failed persistence is reported on screen.

## Controls

Stadium bowling has three separate phases:

1. **Spot:** move the mouse over the lane (or A/D) to place the ball. Click or release Space/Enter to lock.
2. **Curve:** move left/right (or arrows/Q/E) to set hook. Click or release Space/Enter to lock.
3. **Power:** watch the cycling vertical meter. Click or release Space/Enter to bowl at that power.

Backspace returns to the previous step before launch. Locked position and curve do not change while timing power. Each new shot starts at Spot. Mascot Ride keeps its separate pull-back slingshot: grab the course, pull backward and sideways, release; steer with A/D or arrows, boost with Space. Keys 1/2/3 select powers. Escape pauses, M toggles sound, F11 toggles fullscreen.


## Graphics and proportions

Stadium Bowling places the playable lane on the baseball field: striped turf, base paths, dugouts, outfield walls, a seating bowl, crowd, floodlights and stadium landmark silhouettes surround it. There is no generic indoor bowling-hall ceiling or concourse. Wrigley has ivy/brick cues; Tropicana has a roof framework and touch-tank cues. These remain stylized interpretations, not architectural replicas.

Rays Adventure is a separate fantasy course inspired by the supplied Polar/Shrek visuals: navy/aqua surroundings, water channels, rays, bubbles, foliage accents and an original ray riding an inflatable tube. Hold left mouse over the course, pull backward to load the sling and sideways to aim opposite the pull, then release. A longer backward pull increases power. A/D or arrows steer after launch; Space boosts once. Space also remains an alternate timed launch input before launch. The footage establishes the visual slingshot, but exact original mouse-button semantics are inferred.

All playable dimensions are defined in `src/lane_geometry.hpp`; physics and rendering use the same world coordinates. The current arcade baseline has 160 units from foul line to head pin and a 10-unit lane width. The latest distance increase responds to the user's request for pins farther away. Ball/pin sizes and camera framing stay fixed when decoration changes. See `PROPORTIONS.md`. Decoration transforms do not contain the playable lane, ball or rack.

The art remains procedural. Authored models/textures, richer character reactions and more distinct course layouts are still needed to reach the requested commercial-game finish. Logo balls use the MLB team marks. Gameplay is arcade collision simulation, not regulation bowling physics. The 64 ball unlocks, ten stadium unlocks ending with Tropicana, powers, AI rival and saved progression remain in place. Windows runtime and sound playback are unverified here.

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

Headless creates a private display, runs a real native window, then cleans up its server. A smoke test completes both competitors' games, checks roll count and exits nonzero on failure. Smoke mode does not save progression. --arena is a nonpersistent developer preview. Screenshot-only mode uses a fixed 60 Hz step, captures after 90 frames and exits without saving progression. For a repeatable in-motion preview, append `--demo-shot --capture-frame 75`: it launches at frame 30 and captures at frame 75. `--capture-frame N` accepts 1–3600. `artifacts/` is created by the headless helper.

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

## Stadium scoreboards

Blue title panels have been replaced with ballpark scoreboards. Wrigley uses painted green manual panels, white league grids, a clock, flags and lamps inspired by the supplied photograph. Fenway uses a green manual face; other parks and Rays Adventure use dark LED faces with amber numbers. Bowling frame scores and totals update with the match; the decorative baseball grids do not represent live MLB results.

## Easy Windows installation

Run Big-League-Bowling-Setup.exe, choose Install, then Play on the finish screen. The installer adds desktop and Start menu shortcuts and a Windows Installed Apps uninstaller. No admin rights required. Career saves survive uninstall. See installer/README.md for rebuilding. This build is unsigned and Windows execution remains unverified here.

Latest visual reference pass: 160-unit lane run with unchanged width and body sizes. A distant-rack projection test protects the reference composition. Decorative baseball framing, left power strip, player portraits, Bangers arcade lettering and unboxed menu actions replace application-like controls.

## Opening menu and pacing

The illustrated title screen opens Single Player or Mascot Adventure setup. Ball Locker, Scores, Sound, Help and Quit are functional native menu actions; Escape returns from submenus. Enter selects Single Player. Full power reaches the pins in about 3.5 seconds, with unchanged lane size. The floor shader uses defined signed arithmetic to avoid a driver-dependent black-lane artifact.

## Painted arcade menus

All native menus use authored rock/turf frames, chipped slate plaques, illustrated baseball portraits and organic green actions. Match setup follows the compact versus composition of the supplied/official Zombies references. The earlier navy stadium/career dashboard is removed, including its shared panel drawing code. See assets/ARCADE_UI.md for artwork and inspected internet references.
