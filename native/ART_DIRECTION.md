# Current art and control direction

The user's newest setup and trophy screenshots govern ALL native UI. Use original hand-painted rock/turf frames, chipped slate, illustrated baseball portraits and organic green action plaques in a compact arcade composition. No navy website headers, marketing slogans, two-column dashboards or generic cards. The rejected setup layout and its procedural polygon panel renderer have been removed; do not restore them. Title art remains the illustrated baseball poster. See assets/ARCADE_UI.md for the official internet screenshots actually inspected and the original UI textures.

Stadium shot order is explicitly **Spot → Curve → Power**. Confirm each stage separately; position and spin lock before timing release. Backspace edits the previous step. Mascot retains backward-pull/sideways slingshot and in-motion steering/boost. These input mappings must remain distinct.

Preserve native/src/lane_geometry.hpp during graphics work: width 10, foul line 2, head pin 162, run 160. Full power reaches the rack in about 3.5 seconds. Camera presets and regression bounds stay fixed; never shrink the lane to fit new art. See PROPORTIONS.md. Decoration transforms exclude playable lane/ball/rack.

Stadium mode takes place on the field inside the selected baseball stadium. Keep actual sourced stadium/team marks, scoreboards and park landmarks. Mascot mode is a separate Rays fantasy course; the mascot is a continuous three-dimensional ray mesh with swept fins, ivory underside, expressive face and tube, not a sprite. HUD and versus rivals use the baseball-portraits.png atlas (cap player, helmet batter, umpire, ball boy), never primitive object-based faces.

Keep all 64 ball unlocks (30 team, 30 fun, 4 Rays), ten stadiums ending with Tropicana, AI, powers and profile persistence. Commit/push/verify finished work per AGENTS.md. Windows runtime still requires testing on Windows.

Both short Dropbox reference clips and official Steam screenshots have been inspected. Full YouTube inspection and exact original mouse-event semantics have not been verified. Latest three-phase stadium order is the user's explicit instruction. Authored stadium models, course variety and animation remain future work; the prototype is not a finished visual replica.
