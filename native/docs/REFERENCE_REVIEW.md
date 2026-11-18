# Visual reference review — November 18, 2026

The user's Dropbox clip was downloaded and sampled into frames for visual inspection. It is 20.733 seconds, H.264, 2868×1320 at 60 fps; SHA-256 `41bba29fc00be1f4c09b05841d4db2e4ddc11a78a53bc66842d97547be60ff26`. The private sharing URL and original MP4 are not included in this repository.

Observed in the sampled frames:

- 0 seconds: Polar Bowler lane selection for Chill Pin Alley, saturated blue backdrop, large illustrated buttons and warm yellow lettering.
- 4 seconds: tube rider approaching the rack; low camera follows behind the character; the lane sits inside a tall, enclosed ice passage.
- 8 seconds: pin reaction view, blue pinsetter opening and oversized themed sign.
- 12 seconds: rider beside the lane edge, with strong blue side-channel contrast and a compact score strip along the bottom.
- 16 seconds: rider on the lane with a large “BOOM!” reaction overlay and sculpted scenery overhead.

These frames establish framing, palette, scenery density and tube silhouette. They do not establish exact input mapping, collision rules, steering strength, AI behavior or power-up timing. The full requested YouTube clip has not been inspected. The second video is still unavailable; traditional-mode changes use the user's supplied Zombie Bowl-a-Rama screenshot (warm multi-lane interior, close columns, decorated ceiling, blue gutters and side power panel).

## Applied to this build

Mascot mode has a separate enclosed single-lane scene with repeated arches, a tube rider, an aiming band, reflective-looking lane material, side channels and close scenery. The baseball/stadium theme is retained: the final Tropicana course uses an aquarium/ray treatment. Traditional mode has a lower decorated ceiling, closer columns, lamps, neon, lane-side tables and a warm varnished floor. Both use compact dual scorecards, larger illustrated stadium marquees, a wider following camera and scene antialiasing.

The source generates original geometry/materials/sign illustrations. No meshes, textures or video frames from the reference games are shipped. The scenery remains procedural, and the ten courses share geometry with palette/decor variations. Authored art, richer character animation and distinct course layouts remain necessary to approach the requested finish.

## Reproduce the runtime previews

```
python3 native/build.py --sdk-cache /workspace/tools
python3 native/headless.py --play --mascot --arena 9 --screenshot /workspace/yuh/native/docs/tropicana-mascot.png
python3 native/headless.py --play --arena 0 --screenshot /workspace/yuh/native/docs/wrigley-traditional.png
python3 native/headless.py --play --mascot --arena 9 --demo-shot --capture-frame 75 --screenshot /workspace/yuh/native/docs/tropicana-in-motion.png
```

Screenshots use fixed simulation steps and do not write career progress. They are actual native-game captures, not concept renders.
