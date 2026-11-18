Latest signage: stadium scoreboards replace blue title panels. Wrigley follows the supplied green manual-board reference, with a 3D clock, flags and lamps; bowling scores update live. See native/docs/wrigley-scoreboard.png.

# Current handoff

Read AGENTS.md, native/ART_DIRECTION.md, native/PROPORTIONS.md and native/docs/VALIDATION.md first. The user wants stadium mode to be baseball-themed Zombie Bowl-a-Rama on the field of the referenced stadium, and mascot mode to be a Rays-inspired Polar/Shrek fantasy course. The previous indoor hall is gone. Mascot now has backward-pull sling aiming/power; stadium mode retains upward mouse drag power.

The user repeatedly flagged inconsistent proportions. Physics and rendering now share the fixed dimensions in native/src/lane_geometry.hpp; decoration scaling excludes the lane/ball/rack. Latest requested change puts the head pin 80 units beyond the foul line with lane width 10. Preserve these dimensions/camera presets during art-only edits; geometry regression checks guard them. Stage scenery follows the deck position. Both game modes retain progression, AI and powers.

Both short Dropbox videos have been visually inspected; mouse-button semantics remain inferred. Do not claim full YouTube inspection or exact gameplay replication. Windows cross-build is available, but Windows execution is unverified. Always commit, push and verify remote HEAD before calling work preserved.

---

# Historical handoff (superseded where it conflicts with the current notes)

# Codex handoff: Big League Bowling

## Current native reconstruction
Read `native/README.md` and `native/ART_DIRECTION.md` before continuing. The prior uncommitted C++ files were lost in a cloud refresh; the native work has been reconstructed from the conversation. It includes both modes, simulated AI, powers, save/load, 30 MLB team balls + 30 fun team designs + 4 Rays specials, and ten ordered stadium unlocks ending at Tropicana Field. The latest graphics pass includes concourse geometry, signage, lighting shader, animated ray mascot and compact HUD.

Graphics remains priority #1. The November 18 pass uses visually inspected frames from the user's 20.733-second Polar Bowler Dropbox clip and the supplied Zombies screenshot. The second video and full YouTube references remain unavailable; do not claim their mechanics were verified. Mascot mode now has a separate enclosed stadium grotto, inflatable ring, launch band and water channels. Traditional mode has closer columns, inset ceilings and pendant lights. Camera/HUD/signs/materials and scene antialiasing were refined. Read native/docs/REFERENCE_REVIEW.md and native/docs/VALIDATION.md. Authored art and more distinct stadium courses remain necessary.

The user explicitly requested durable preservation after patches. Follow `AGENTS.md`: commit and push completed source/assets/tests/docs, then verify remote HEAD. `native/docs/VALIDATION.md` records current verification once complete. Keep caches and generated binaries out of source history. Windows packaging is reproducible with `native/build.py`; running the EXE on Windows remains unverified.

## Original browser prototype handoff


## User goal
Build a baseball-themed 3D arcade bowling game inspired by Elf Bowling, Zombie Bowlarama, Polar Bowler, and Shrek Bowling, with MLB teams and stadiums. Deliver a downloadable Windows desktop game. Continue development in another Codex session using this branch.

## Implemented
- Three.js procedural baseball, cap-wearing pin characters, lane, generic ballpark, crowd and sign; following camera, pin idle and topple animation.
- Aim, position, power, curve, deterministic 2D collisions, ten-frame scoring including tenth-frame bonus rolls.
- 30 team/stadium text entries; only some teams have individual palettes. All venues share one stylized stadium.
- Standalone offline HTML packaging and Windows x64 Electron packaging. Desktop main process disables Node integration and uses sandbox/context isolation. F11 fullscreen.

## Run and build
Run `npm ci`, `npm test`, then `npm start` (Python 3 required). Open localhost:3000.
`npm run build:pc` produces the self-contained HTML folder.
`npm run build:windows` builds the Windows desktop folder under dist/windows using Electron 40.0.0. ZIP that entire folder for release. In environments using an HTTP proxy, Node 24 may need NODE_USE_ENV_PROXY=1. The packaging download cache currently points to /tmp/electron-cache; adapt for Windows hosts.
Browser smoke tests use /usr/bin/chromium, so adapt the executablePath if needed. `test/download.cjs` checks bundled HTML offline through setContent; direct file navigation was blocked by the managed test browser policy.

## Verification and limitations
Nine logic tests passed. Browser smoke tests passed for WebGL startup, team choice, shot completion, restart, and mobile rendering. Offline bundled HTML gameplay passed. Windows PE executable and ZIP contents were checked, but the executable was NOT launched on Windows. Build is unsigned. This remains a prototype, not a finished recreation of the reference games. No audio, unique stadium models, saved progress, or full 3D rigid-body physics.

## Delivery issue / next action
The user cannot download workspace or sandbox links in browser Codex. They provided https://github.com/jaysleftfoot-hue/yuh.git to host a GitHub Release. GitHub CLI API authentication previously returned 401, while git repository reads succeeded. This handoff requests committing and pushing all source to a dedicated branch. Rebuild the Windows ZIP and upload as a release asset once authenticated access is available. Do not put the 132 MB ZIP into Git source history.

## Original workspace artifacts
The original task workspace contains /workspace/baseball-bowling/dist/Big-League-Bowling-Windows-x64.zip (~132 MB), plus source and dependencies. Those files may not survive a move to another environment; source and lockfile here reproduce the build. Generated outputs and node_modules are intentionally excluded from Git.

Latest follow-up: rendered lane proportions use the 60 ft / 41.5 in ratio, converted from normalized simulation coordinates. Shot timing/physics remain arcade-style. HUD has simpler panels/buttons, horizontal powers, and hides shot controls in motion. See ART_DIRECTION.md and VALIDATION.md for scope and checks. The prior artifact backup is the earlier graphics build; rebuild the latest Windows ZIP from source.

Latest Zombies follow-up: second Dropbox clip successfully inspected (26.333 s). Traditional now uses mouse placement, upward drag power, sideways drag aim and release-to-launch, with a vertical meter instead of sliders. Mouse events are inferred, not verified from video. Traditional hall depth was restored to compressed arcade proportions after the regulation-ratio scene stretch looked wrong. Mascot remains separate. See the latest sections in ART_DIRECTION.md and REFERENCE_REVIEW.md.

Latest detail pass: slim 49-pixel score strips replace chunky scorecard tiles. The huge center signs are removed from traditional and mascot environments; original neon bowling outlines and small side stadium plaques take their place. See native/docs/scorecard-result.png for a completed scorecard preview.
