# Graphics and gameplay direction

Graphics is priority #1. The user wants the rich, playful 2005–2010 arcade bowling look in the supplied Polar/Shrek/Zombie screenshots: low following camera, visible rolling ball/mascot, deep themed scenery, expressive characters, glossy materials and chunky illustrated controls.

## Reference access — November 18, 2026

- Traditional mode: https://www.youtube.com/watch?v=b8WkE56ZC9s — first minute (0:00–1:00).
- Mascot mode: https://www.youtube.com/watch?v=CmlZsIFJ0sA — entire video.
- Use both as graphics references too. Inspect aim/release controls, launch, steering, boost, obstacles, pickups, camera, scoring, AI/powers and reaction timing before claiming matching behavior.

The user supplied a Dropbox MP4 that downloaded successfully: 20.733 seconds, 2868×1320, 60 fps. Extracted frames show Polar Bowler's Chill Pin Alley selection, tube rider, enclosed blue ice scenery, pin impacts and following camera. Only this short clip has been visually inspected; it is not the entire YouTube video. The second clip remains inaccessible (the original MP4 uploads exceeded the 32 MiB transfer cap, and YouTube requests returned proxy CONNECT 403). Traditional-mode art therefore uses the supplied Zombie Bowl-a-Rama screenshot, not a claim of having watched the Zombies video. Existing controls/AI/powers are not reference-verified mechanics. See docs/REFERENCE_REVIEW.md for observations and limits.

## Roster and progression requirements

64 ball designs means exactly **30 MLB team balls + 30 fun team-inspired designs + 4 Rays specials**. The user is a Rays fan. Ten stadium alleys; Tropicana Field must be the final unlock. Traditional and mascot modes both need single-player AI competition and powers.

## Reconstructed graphics pass

- Multi-lane concourse, pinsetter masks, balconies/railings, brickwork, trusses, ceiling panels, snack tables and crowd.
- Readable stadium marquees generated in-engine; Rays signs at Tropicana.
- Material shader: warm key light/cool fill, view-dependent highlights, fog, wood grain/seams, approximate pin/ball contact shadows and stylized ceiling-light highlights. This is not ray tracing or full reflection rendering.
- Original ray: curved volume wings, stitched sled and metal runners, jersey, cap, blinking eyes, head turn while aiming, segmented tail.
- Pin jersey patches, eyebrows, cap, mitt hands and shoes.
- Reduced bottom HUD, right-side power icons, framed scoreboard.

## November 18 graphics pass

- Mascot mode now has a separate enclosed single-lane environment: irregular curved walls, three illuminated archways, side water channels, animated water/caustic materials, rail lamps and stadium marquees. Wrigley/Fenway/Seattle add foliage; Tampa Bay/Oracle/Kauffman have decorative rays and rising bubbles. These are stadium-themed arcade courses, not real stadium replicas.
- Ray mascot rides a modeled inflatable ring with a raised rim, two-tone rubber, grab handles and perimeter rope. Smoothed, mirrored wings and cosmetic banking follow horizontal velocity. A visible launch band stretches with selected power; controls/physics remain unchanged.
- Traditional mode gains close brick columns, lower inset ceiling panels, pendant lights, lane-side tables/cups and neon accents. Existing neighboring lanes, concourse, crowd and stadium landmarks remain.
- Revised wood grain/seams, broader floor highlights and mottled grotto materials. Highlights are stylized lighting, not true scene reflections. FXAA affects the 3D scene only; the interface remains crisp. Font/sign mipmaps reduce distant shimmer.
- Wider following camera; launch framing keeps the tube above the controls. Two compact scoreboards replace the tall stacked scoreboard/header during play. Stadium signs use original baseball/bat artwork and a stronger title treatment.

Remaining art work: authored character meshes/textures, varied course layouts beyond palette/decor changes, a better character rig and reactions, fuller environment shadows, more illustrated UI, and access to the second reference video. The result is closer in framing and environment density, but still not near-identical to the commercial games.

## Recovery record

A cloud refresh removed the prior uncommitted native files. The native implementation was reconstructed from code and requirements recorded in the conversation, then rebuilt/tested. It is not a byte-identical filesystem restore. Preserve this reconstruction in the remote Git branch; record current verification in docs/VALIDATION.md. Source screenshots in docs/ are newly captured evidence, not recovered old files.

## Lane proportions and HUD follow-up

The rendered foul-line-to-head-pin distance now uses 60 feet / 41.5 inches (17.35 lane widths). The simulation still uses normalized arcade coordinates and existing shot timing; this is a presentation conversion, not a regulation physics rewrite. Rack depth is preserved after the head pin to avoid stretching the pin triangle. Characters remain stylized. Small scenery props retain their depth while the lane and enclosing scenery extend longitudinally. The following camera uses converted positions.

Removed riveted panel borders and the right-side power tower. Powers are compact buttons above the shot bar; shot controls disappear during travel/reaction. Menu panels share the simpler treatment. The launch camera pitches down to keep the rider clear of the bottom controls.
