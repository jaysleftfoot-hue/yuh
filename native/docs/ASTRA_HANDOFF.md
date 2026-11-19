# Big League Bowling art handoff

Start with one full Wrigley frame from the normal bowling camera. The existing stadium visuals and pickup markers show the game states that need authored graphics. Use the original baseball arcade direction in [ART_DIRECTION.md](../ART_DIRECTION.md). Keep the shared dimensions and camera presets in [lane_geometry.hpp](../src/lane_geometry.hpp), and keep decoration outside the playable lane transform.

## Implemented art pass

[Native camera captures of Wrigley, Mascot Ride and Tropicana](ballpark-art-preview.jpg)

The game loads original generated paintings and emblems from `native/assets/ballpark-art`. Wrigley and Tropicana use curved painted backdrops behind the live scoreboard. The aquarium uses illuminated exhibit windows, a textured tunnel and swimming ray meshes. Lane pickups share the six HUD badges. A separate manta medallion identifies challenge tokens. Rival quips have portrait badges, and the Tropicana celebration and unlock screen use the sunburst painting.

## Playable scenes

Build with `python3 native/build.py`. These native-window captures use a fixed simulation clock and leave career saves untouched.

| Scene | Command after `python3 native/headless.py` |
| --- | --- |
| Wrigley from the foul line | `--play --screenshot artifacts/wrigley.png --capture-frame 5` |
| Strike replay with board video | `--play --demo-strike --screenshot artifacts/strike.png --scoreboard-screenshot artifacts/strike-board.png --capture-frame 465` |
| Fenway 7–10 conversion | `--preview-split --demo-shot --screenshot artifacts/fenway-split.png --capture-frame 440` |
| Mascot route and pickups | `--play --mascot --screenshot artifacts/touch-tank.png --capture-frame 5` |
| Tropicana from the foul line | `--play --arena 9 --screenshot artifacts/tropicana.png --capture-frame 5` |

See the [Wrigley replay feed](wrigley-replay-board.jpg) and the [Fenway conversion](fenway-split-replay.jpg) as rendered by the current build.

To see a manual board flip, let a replay finish or skip it with Space. To trigger the Tropicana event, bowl three consecutive strikes at Tropicana in Stadium mode. The career unlock announcement fires after winning at Yankee Stadium when that victory opens the last park. In setup, the Fenway split drill is available after unlocking Fenway and selecting Stadium mode.

## Graphics work

| Scene or asset | Existing C++ and shader interface |
| --- | --- |
| Wrigley ivy, clock, brick, trusswork, floodlights and stands | [Art::stadiumScenery and Art::landmarks](../src/art.hpp) own the world placement. Keep signs readable from the foul line. |
| Mechanical scoreboard, working clock art and replay surround | `Art::updateScoreboard` has old/new string values for each score tile and runs a 0.42-second transition. The 512 × 288 replay video comes from `replayFeed` in [main.cpp](../src/main.cpp). Preserve the deck-only scene for the video render so it never samples its own stadium board. |
| Rays tunnel, curved aquarium glass, swimming rays and caustics | `Art::mascotScenery` has timed free-swimming positions beyond the rails. Replace the temporary ray silhouettes and add the glass within the scenery pass. [lighting.fs](../assets/lighting.fs) already sends scene time to the animated lane material. |
| Slingshot, turn lean and gutter wipeout | [MascotPose](../src/presentation.hpp) exposes normalized pull, lateral pull, release recoil, bank and wipeout. `Art::rider` and the sling endpoints consume the pose. Attach visuals to the same pose while the physics body keeps its canonical radius and position. |
| Trop roof, catwalks, stadium board, sunbursts, unlock reveal | `StadiumCue::Turkey` and `StadiumCue::TropicanaUnlocked` signal the moments in both the match UI and board. `cueSeconds` carries the countdown and `boardLight` passes intensity and deck origin to the lane shader. Replace the temporary celebration with an original Rays animation. |
| Six pickup props, HUD badges and three challenge tokens | `PowerPickups::items` carries power type, lane position and collection state. Keep the lane props and HUD badge keyed to the same `Power` value. `MascotTokens::layout` defines the fixed three-token route and tracks each collected token independently. |
| Rival reactions and challenge rewards | `rivalReaction` keys strike, spare and gutter lines by style and bowler. The game also signals close finishes and first-time rewards. Use the existing portrait atlas as the starting point for reaction art. |

## Pickup reference and tuning

The two reference games used both helpful items and opponent tricks. *Elf Bowling 7 1/7* included a speed boost, gutter removal, ball shrinkage and loss-of-control items. *Zombie Bowl-o-Rama* used lane collectibles that stock powers for subsequent turns and combined ball, lane and pin effects. Big League Bowling uses the shared play pattern with its own power names and visuals. [Elf Bowling reference](https://www.mobygames.com/game/34717/elf-bowling-7-17-the-last-insult/) · [Zombie Bowl-o-Rama gameplay reference](https://www.mobygames.com/game/43571/zombie-bowl-o-rama/) · [Official Zombie Bowl-o-Rama listing](https://store.steampowered.com/app/32160/Zombie_BowloRama/)

Each bowler rolls independently for at most one floating medal per frame, including spare and bonus attempts. The spawn chance rises after each empty frame and resets after each spawn. All-Star caps medals at two per bowler and Hall of Fame caps them at one. See the difficulty table in the [native README](../README.md). Pickups use swept collision against the ball/rider radius, award stock to the current bowler and cap inventory at three per kind. Challenge tokens have a fixed course and require three in the same shot. Keep the two kinds of collectibles visually distinct.
