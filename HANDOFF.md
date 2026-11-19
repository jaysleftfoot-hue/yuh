# Current milestone

Ballpark challenges, home-rival styles, pin-deck replays, Tropicana events, mascot motion and six randomized power-ups are implemented in C++. See [Astra art handoff](native/docs/ASTRA_HANDOFF.md) for the playable scenes, render hooks and planned visual work.

The 64-ball locker now ships separate club logo, illustrated club, and Rays special wraps. The supplied MLB reference kit guides marks and themes. Left/right arrows rotate the selected locker preview. The existing unlock slots and XP thresholds still apply.

Current character assets are the textured GLBs in native/assets/models. Editable geometry and paint live in native/art. The ready and impact previews are native/docs/ray-model.png and native/docs/pin-model.png.

Latest gameplay pass adds a live standing-pin display and Rookie, Pro and All-Star rivals. Click the AI label in Ballpark Showdown to choose a tier for either mode. Pin indicators use the current simulation rack and its original pin IDs. Match flow uses named screen and turn states. See native/docs/VALIDATION.md for checks.

Latest arcade UI / controls milestone (supersedes older UI and input notes below): all native panels now use original painted rock/turf, chipped slate and green organic actions. Compact versus setup replaces the rejected navy dashboard; locker, help, scores, HUD, pause and results follow the same art. Internet references and provenance: native/assets/ARCADE_UI.md. Stadium bowling now uses separate Spot → Curve → Power confirmations. Mouse motion / A-D place; click locks; mouse / arrows set curve; click locks; third click times the power meter and releases. Space/Enter confirms, Backspace goes back. Mascot slingshot remains separate. No geometry/camera/speed changes.

Latest character pass: vector-style baseball HUD portraits replace primitive faces; rivals cycle helmet batter/umpire/ball boy by stadium. Actual ray 3D body rebuilt in native/src/ray_model.hpp with swept fins and ivory underside; art.hpp supplies face, tail, cap and tube. No lane, speed or input changes. See native/assets/CHARACTER_ART.md.

Latest: illustrated native title screen with real bottom menu controls. Screen0 title, 1 gameplay, 2 locker, 3 match setup, 4 career, 5 help. Enter/Single Player goes to setup; Escape returns to title from submenus. Full-power travel tuned to ~3.5s with lane dimensions unchanged. Signed floor pow() expressions replaced to address driver-dependent black half-lane. Windows graphics fix requires user-machine confirmation.

Latest milestone uses the supplied graveyard screenshot as composition reference: 160-unit lane and original baseball arcade HUD, no boxed gameplay/menu buttons. Bangers display font and OFL are packaged. Read latest sections of native/PROPORTIONS.md and native/ART_DIRECTION.md before editing.

Latest: native/installer/setup.nsi builds a per-user Windows installer with shortcuts and uninstaller. Read native/docs/CONTROL_REFERENCE.md for the restored reachable tube drag, timed/drag stadium power and corrected low Zombies-like camera. native/PROPORTIONS.md latest section supersedes earlier camera experiments.

Latest stadium identity pass: actual sourced stadium wordmarks and MLB marks in native/assets/parks (provenance in SOURCES.md), layered video graphics and steel trusses. Tropicana has a wide screen, catwalk and orange landmark. Stadium ready camera intentionally reframed; world geometry remains 80×10. See native/PROPORTIONS.md for the projected-view regression guard. Existing ten-park roster unchanged; Citi, Comerica and Rate photos inform presentation only.

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
