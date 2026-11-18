# Current direction — stadium bowling and Rays adventure

The references specify the rendering style and gameplay feel, not literal scenery to transplant into every mode. The user explicitly corrected the indoor-hall approach: stadium mode must feel like a baseball-themed Zombie Bowl-a-Rama, with a bowling lane in the middle of the selected stadium. Mascot mode should feel like a Tampa Bay Rays-inspired Polar/Shrek bowling game.

## Current implementation

- Stadium mode: single lane on a baseball diamond, turf and base paths, dugouts, continuous stands, crowd, floodlights, outfield walls and stadium-specific landmark silhouettes. Wrigley ivy/brick and Tropicana roof/touch-tank features are examples. The former enclosed hall has been removed.
- Mascot mode: a distinct navy/aqua Rays fantasy course, tube rider, water, rays/bubbles, vegetation variants and a visible slingshot. Pull backward for power, sideways to aim opposite the pull, release to launch. Steering and boost remain available after launch.
- The slim score strips and stadium drag-to-power control remain. Stadium mouse drag goes upward; mascot sling drag goes downward. Do not accidentally merge the two mappings.
- Canonical playable dimensions and camera presets live in `src/lane_geometry.hpp`. Simulation and rendering now share world coordinates. Scenery transforms exclude the lane, ball and rack. The user requested farther pins; foul-line-to-head-pin distance is now 80 units, up from 42, with width and body sizes fixed. Read PROPORTIONS.md before changing geometry.

## References actually inspected

The user's Polar Dropbox clip is 20.733 seconds; the Zombies Dropbox clip is 26.333 seconds. Frames from both were inspected, including dense samples of Zombies' placement/power sequence. The Polar clip shows the inflatable rider, sling presentation, following camera and enclosed themed scenery. The Zombies clip shows lateral placement indicators, a vertical power meter and following camera. Neither recording exposes button events; exact mouse semantics are inferred. Full YouTube videos remain unviewed. No original meshes, game textures, private sharing URLs or MP4s are shipped.

## Requirements retained

Exactly 30 MLB team balls, 30 fun team-based designs and four special Rays balls. Ten stadium unlocks with Tropicana Field last. AI opponent, powers and saved progression. Preserve completed source/assets/tests/docs by commit, push and remote-HEAD verification per AGENTS.md.

## Remaining work

This is still procedural art, not a finished visual clone. Add authored stadium/character assets, better animation, lighting/shadows, illustrated UI and varied mascot courses. Keep proportion changes separate and intentional: do not change lane scale to compensate for a background or camera problem. Historical notes in docs/ describe earlier experiments and must not override this current direction.

## Stadium scoreboards

Blue title panels have been replaced with ballpark scoreboards. Wrigley uses painted green manual panels, white league grids, a clock, flags and lamps inspired by the supplied photograph. Fenway uses a green manual face; other parks and Rays Adventure use dark LED faces with amber numbers. Bowling frame scores and totals update with the match; the decorative baseball grids do not represent live MLB results.
