# Codex handoff: Big League Bowling

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
