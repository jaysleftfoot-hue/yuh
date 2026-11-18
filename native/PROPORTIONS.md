# Fixed playable proportions

`src/lane_geometry.hpp` is the single source for the playable layout. These are arcade world units, not a claim of regulation bowling dimensions.

| Property | Current baseline |
|---|---:|
| Lane width | 10 |
| Foul line Z | 2 |
| Head pin Z | 82 |
| Foul-line-to-head-pin distance | 80 |
| Normal ball radius | 0.85 |
| Heavy ball radius | 1.10 |
| Pin radius / height | 0.55 / 3.28 |
| Pin spacing X / row depth | 2.65 / 2.295 |
| Deck exit Z | 92 |
| Surface end Z | 95 |

The distance was intentionally increased from 42 to 80 after the user said the ball still looked too close to the pins. Width, ball/rack sizes and camera presets were held fixed for that change. This supersedes the abandoned renderer-only regulation-ratio experiment.

Simulation and rendering now use identical Z coordinates. The only playable X conversion changes the origin and handedness (`halfWidth - x`) to match a camera looking down positive Z; it does not scale positions. The rack is built from the shared spacing, and the pin model scale derives from the collision radius/height. The ball renderer takes its radius from the simulation. Placement controls clamp to the same launch bounds as physics.

Scenery may use an authored coordinate transform, but it must be popped before drawing the playable surface, ball and rack. Stadium stands and the mascot course's back scenery extend using the deck-position-derived scenery offset. Never stretch playable geometry to fit a background.

The existing rules suite checks the baseline length/width, body sizes, spacing, camera fields of view, rack containment and launch bounds. The campaign suite checks both drag mappings, opposite-direction sling aim and power limits. Intentional layout changes require updating the shared definition, its regression checks and this document; graphics-only changes must preserve them. Validate a launch view and a pin-impact view in both modes before preserving a graphics milestone.

## Camera correction after stadium references

World length remains 80, width 10, all body sizes unchanged. Stadium ready camera is intentionally raised from 3.8 to 6.5 and moved back from -8 to -12, targeting height 0.6 with unchanged 55-degree FOV. This exposes more lane floor and reduces the oversized foreground ball. Ready framing snaps to the preset instead of retaining a menu/previous-shot interpolation. Mascot and follow cameras are unchanged. Tests now lock every ready camera component and require 285–305 screen pixels between foul line and head-pin base at 1280×800.
