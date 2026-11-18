# Control and scale reference review

Reviewed the supplied Zombies Dropbox video (26.333s), its dense placement/power sequence, and the Polar Dropbox video (20.733s). Also inspected official screenshots and game description from https://store.steampowered.com/app/32160/ and its appdetails API. No exact world measurements or mouse events are exposed; mappings below are adaptations, not verified original source behavior.

Zombies shows a low partly cropped ball, distant rack, horizontal placement arrows, an upright filling meter, then a following camera. Stadium mode now uses a lower, closer ready camera to reproduce that composition without another world-length change. Move mouse to position; hold to build power, drag upward to set it directly, sideways to aim, release to bowl. The visible meter shows either path. Keyboard fallback remains.

Polar shows a pulled inflatable rider and sling followed by a trailing camera. Grab the tube or open course, pull down/back and sideways, release; A/D or arrows steer, Space boosts. The old Y<630 input gate excluded the actual tube around Y=700: fixed. The sling rests until grabbed and visibly stretches with pull. Buttons in the side HUD are excluded from the course hit area. Tests cover the tube hit and HUD exclusion.

The previous flat panels are now beveled metal plaques with grain, rivets and segmented meters; this is still procedural artwork, not the original games' illustrated assets. Physics and original unlock/AI/power systems remain intact.
