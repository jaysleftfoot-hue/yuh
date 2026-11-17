# Graphics and gameplay direction

Graphics is priority #1. The user wants the rich, playful 2005–2010 arcade bowling look in the supplied Polar/Shrek/Zombie screenshots: low following camera, visible rolling ball/mascot, deep themed scenery, expressive characters, glossy materials and chunky illustrated controls.

## Exact video requests — still unviewed

- Traditional mode: https://www.youtube.com/watch?v=b8WkE56ZC9s — first minute (0:00–1:00).
- Mascot mode: https://www.youtube.com/watch?v=CmlZsIFJ0sA — entire video.
- Use both as graphics references too. Inspect aim/release controls, launch, steering, boost, obstacles, pickups, camera, scoring, AI/powers and reaction timing before claiming matching behavior.

Neither video has been watched here. Requests returned proxy CONNECT 403 even after the runtime settings reported unrestricted networking. The two MP4 uploads exceeded the executor's 32 MiB transfer limit. Retry YouTube after a meaningful access change, or inspect smaller uploaded clips. Existing steering/boost/power-ups predate these references and must not be described as copied from inspected footage.

## Roster and progression requirements

64 ball designs means exactly **30 MLB team balls + 30 fun team-inspired designs + 4 Rays specials**. The user is a Rays fan. Ten stadium alleys; Tropicana Field must be the final unlock. Traditional and mascot modes both need single-player AI competition and powers.

## Reconstructed graphics pass

- Multi-lane concourse, pinsetter masks, balconies/railings, brickwork, trusses, ceiling panels, snack tables and crowd.
- Readable stadium marquees generated in-engine; Rays signs at Tropicana.
- Material shader: warm key light/cool fill, view-dependent highlights, fog, wood grain/seams, approximate pin/ball contact shadows and stylized ceiling-light highlights. This is not ray tracing or full reflection rendering.
- Original ray: curved volume wings, stitched sled and metal runners, jersey, cap, blinking eyes, head turn while aiming, segmented tail.
- Pin jersey patches, eyebrows, cap, mitt hands and shoes.
- Reduced bottom HUD, right-side power icons, framed scoreboard.

Remaining art work: richer authored textures/models, distinct stadium layouts, better character rig and reactions, environment shadows and lighting, and close comparison with the requested footage once available. Do not describe this as near-identical to the reference games.

## Recovery record

A cloud refresh removed the prior uncommitted native files. The native implementation was reconstructed from code and requirements recorded in the conversation, then rebuilt/tested. It is not a byte-identical filesystem restore. Preserve this reconstruction in the remote Git branch; record current verification in docs/VALIDATION.md. Source screenshots in docs/ are newly captured evidence, not recovered old files.
