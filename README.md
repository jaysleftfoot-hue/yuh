# Big League Bowling

Run `npm install`, then `npm start`; open http://localhost:3000. Run `npm test` for nine rules/physics tests. `npm run test:browser` runs Chromium smoke coverage (expects `/usr/bin/chromium`).

## Current game

Three.js renders original procedural 3D assets: a stitched baseball, lathed bowling-pin characters with baseball caps and faces, wooden lane and gutters, turf stripes, grandstands, crowd figures, floodlight structures, and a ballpark sign. Asset factories live in `src/assets3d.js`; these are real runtime mesh geometry, not pre-rendered images or exported GLB files.

The camera follows the ball. Pins sway while idle and topple on impact. A brief reaction pause preserves the result before the next rack. Controls cover position, aim, power, and curve. Ten-frame scoring includes strikes, spares, and tenth-frame bonuses. All 30 clubs are selectable by text; a subset has custom palettes and others currently use a common blue palette.

## Architecture and limits

`src/renderer.js` reads the deterministic lane simulation in `src/physics.js`. `src/scoring.js` owns bowling rules. The graphics are 3D, while collisions remain a simplified 2D arcade approximation. Stadium labels use team metadata; all teams currently share one original stylized stadium, not architectural replicas. No official logo assets are included.

The visual direction uses a low behind-ball view and playful pin characters inspired by classic arcade bowling. Character voice lines, elaborate comic reactions, audio, stadium-specific obstacles, exported model files, and unique ballpark models are not implemented. Browser tests cover rendering startup, team selection, completing a shot, restarting, and a mobile viewport.

## Portable PC download

`dist/Big-League-Bowling-PC.zip` contains a self-contained HTML game, Windows launcher, instructions, and Three.js license. Extract it and open `Big League Bowling.html`. No server or internet connection is required. Rebuild the folder with `npm run build:pc`. This is a browser-based portable release, not a native executable.

## Windows desktop game

`dist/Big-League-Bowling-Windows-x64.zip` contains the Electron desktop release for Windows 10/11 x64. Extract all files and launch `Big League Bowling.exe`. Keep its supporting files together. F11 toggles fullscreen; Escape leaves fullscreen. No separately installed browser or development tools are needed.

Rebuild using `npm run build:windows`, then ZIP the resulting folder in `dist/windows`. The desktop executable is unsigned. Packaging, archive contents, executable architecture, and game logic were checked on Linux; launching the executable on Windows remains unverified.
