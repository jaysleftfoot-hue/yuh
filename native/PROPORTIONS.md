# Canonical playable proportions

The user's latest Zombies graveyard screenshot is the composition reference: a long tapering lane, tiny distant rack and low foreground ball. Exact original world units are unknown. The current intentional arcade baseline is:

| Property | Stadium world units |
|---|---:|
| Lane width | 10 |
| Foul line | 2 |
| Head pin | 162 |
| Playable run | 160 |
| Deck exit / surface end | 172 / 175 |
| Normal / heavy ball radius | 0.85 / 1.10 |
| Pin radius / height | 0.55 / 3.28 |
| Rack spacing X / row depth | 2.65 / 2.295 |

Stadium uses these dimensions from `src/lane_geometry.hpp`. Rendering and simulation share world coordinates; only X handedness is converted. Decorative transforms must end before the playable lane, ball and rack are rendered. Stadium seating and mascot course extend around the playable layout.

Stadium ready camera: height 3.8, Z -4.5, target height 1.1, vertical FOV 55. That camera is regression-tested. At 1280×800 the stadium rack width must project to 38–46 pixels and foul-line-to-head-pin screen distance to 390–440 pixels. This catches visual shortening that a world-length check alone misses.

## Mascot Ride

A centered 4:3 gameplay frame from the supplied Polar Bowler clip at 3.1 seconds puts the feet of the front pins about 41% down the viewport. The sling posts span about 95% of the viewport height in screen pixels. The revised Mascot layout uses a 144-unit run, a head pin at 146, a deck exit at 156 and a surface end at 159. The lane is 10 units wide. The ready camera sits 3.70 units high at Z −7.45, looking down to −13.35 at the rack, with a 53° vertical FOV. This gives a front pin projection around 42% from the top in our wider gameplay window. The clip provides screen proportions. The world units are the game’s own scale.

The layout comes from the live simulation and drives the visible lane, deck, rack, route depths, replays and surrounding aquarium scenery. The projection test guards the rack, foul line and sling width in the Mascot opening view. Stadium uses its 160-unit run and independent camera.

## Throw pacing

Launch speed interpolates from 160/6 units per second at minimum power to 160/3.5 at full power. A straight full-power throw reaches the Stadium rack in about 3.5 seconds and the Mascot rack in about 3.15 seconds. In Mascot Ride, sling tension sets forward speed, and steering changes lateral movement.
