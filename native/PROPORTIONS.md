# Canonical playable proportions

The user's latest Zombies graveyard screenshot is the composition reference: a long tapering lane, tiny distant rack and low foreground ball. Exact original world units are unknown. The current intentional arcade baseline is:

| Property | World units |
|---|---:|
| Lane width | 10 |
| Foul line | 2 |
| Head pin | 162 |
| Playable run | 160 |
| Deck exit / surface end | 172 / 175 |
| Normal / heavy ball radius | 0.85 / 1.10 |
| Pin radius / height | 0.55 / 3.28 |
| Rack spacing X / row depth | 2.65 / 2.295 |

Both modes use these dimensions from `src/lane_geometry.hpp`. Rendering and simulation share world coordinates; only X handedness is converted. Decorative transforms must end before the playable lane, ball and rack are rendered. Stadium seating and mascot course extend around the playable layout.

Stadium ready camera: height 3.8, Z -4.5, target height 1.1, vertical FOV 55. Mascot ready camera: height 4, Z -7, target height -1, FOV 53. All components are regression-tested. At 1280×800 the stadium rack width must project to 38–46 pixels and foul-line-to-head-pin screen distance to 390–440 pixels. This catches visual shortening that a world-length check alone misses.

The latest deliberate change doubled the playable run from 80 to 160 without changing width, body sizes or cameras. It supersedes previous shorter layouts and the abandoned raised-camera experiment. Physics tests and smoke bounds derive their simulation duration from the shared shot timeout. Graphics-only changes must not modify these constants; intentional changes require updated tests, docs and user-facing explanation. Check launch and pin-impact views in both modes.

## Throw pacing

Lane dimensions and camera presets remain unchanged. Launch speed interpolates from 160/6 units per second at zero power to 160/3.5 at full power. Straight full-power throws reach the head pin in approximately 3.5 seconds; power and aim still affect travel time, and mascot boost can shorten it. Simulation tests measure the actual traversal and check rack contact to catch tunneling.
