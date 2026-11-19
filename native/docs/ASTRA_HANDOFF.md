# Big League Bowling graphics handoff

Continue on `codex/big-league-bowling` in `native/`. Start with Mascot Ride and the original Rays aquarium theme. The current build has separate Straight and Power rules, three-dimensional ride coins, and the shorter Mascot lane fitted to the opening frame of the supplied clip. Use the original baseball arcade art direction for new props, animation and effects. [Art direction](../ART_DIRECTION.md)

[Aquarium graphics pass in play](aquarium-ride.jpg) shows the loaded sling, both power pickups, live impact, strike replay and the Wrigley pin deck. The [earlier QA captures](mascot-qa-preview.jpg) include the Power Ride result and combined match scoreboard.

## Play the current build

Build with `python3 native/build.py --sdk-cache /workspace/tools` on the cloud machine. On a fresh Linux checkout, use `python3 native/build.py` to download the pinned SDK. Launch `./dist/native-linux/big-league-bowling` in a graphical session.

In **Mascot Adventure**, choose **Power Ride** or **Straight Ride** below the rival. Move over the lane to position the tube. Grab and pull straight back to set strength, release to start forward motion, then steer with the mouse or A/D. Space can draw the sling and Backspace cancels a draw. The two Power Ride coins appear after loading the sling. Wave Crash is armed until first contact with a pin. The 2× coin adds one point per pin knocked down on that delivery. Frame marks follow regular bowling and the aquarium board uses the combined match total. The three manta coins are the separate Touch Tank challenge.

Use the existing 144-unit Mascot run, 160-unit Stadium run, shared 10-unit lane width and respective opening cameras when changing the scenery. Rails, rack, pickups and rider use simulation coordinates. Keep decoration outside that transform. [Measured lane framing and regression bounds](../PROPORTIONS.md)

## Aquarium graphics pass

- **Pickups.** The Wave Crash crest sits in a scalloped sea-star setting. The violet 2× emblem sits in a diamond. Both rock toward the rider and cast a tinted marker at their lane position. Collecting either draws a short burst around the tube. The three round gold manta medals still mark the Touch Tank challenge.
- **Sling and rider.** Piped leather straps close around a padded yoke on the tube. Drawing it compresses the inflatable and folds the fins. On release, the sling rebounds and the rider pitches forward. During a carve, the two fins flex asymmetrically and a narrow wake follows the tube.
- **Impact and replay.** A broken crown of water, foam and airborne droplets starts at the point of the first powered pin hit. The effect reads the sampled shot time. Replay frames also retain the rider pose for the main view and scoreboard feed. Fallen pins cast a contact shadow along the direction of the fall.
- **Aquarium.** The original painted exhibit behind the deck features a sunken Gulf Coast ballpark, a diving bell and spotted eagle rays. Lateral windows, swaying seagrass, riveted arches and shifting light through the overhead water give the course depth as the tube passes.

The pickup positions and effects live in `RidePickups`. `MascotTokens` owns the fixed three-coin Touch Tank route. Stadium powers use `PowerPickups::item` and the Dugout Equipment case. `Art::updateScoreboard` owns the outfield display and the 512×288 deck-only replay feed. [Ride presentation state](../src/presentation.hpp) · [Renderer](../src/art.hpp)

## Repeatable captures

Run these from the repository root after a Linux build. Preview and smoke commands leave career saves alone. `--arena 9` previews Tropicana directly.

```sh
python3 native/headless.py --play --mascot --arena 9 --capture-frame 4 --screenshot artifacts/mascot-ready.png
python3 native/tests/ui_flow.py mascot-pull
python3 native/headless.py --demo-wave --capture-frame 253 --screenshot artifacts/reef-wave.png
python3 native/headless.py --demo-wave --capture-frame 450 --screenshot artifacts/reef-replay.png
python3 native/headless.py --play --mascot --arena 9 --demo-shot --capture-frame 310 --screenshot artifacts/mascot-impact.png
python3 native/headless.py --smoke --mascot --arena 9 --screenshot artifacts/power-result.png --scoreboard-screenshot artifacts/power-board.png
python3 native/headless.py --smoke --mascot --straight-ride --arena 9
python3 native/headless.py --smoke --capture-frame 2300 --screenshot artifacts/stadium-impact.png
```

The draw capture is `artifacts/arcade-mascot-pull.png`. To check the Stadium launch and the three-shot setup phases, also run `python3 native/headless.py --play --capture-frame 4 --screenshot artifacts/stadium-launch.png` and `python3 native/tests/ui_flow.py phases`. [Stadium scoreboard art](arcade-scoreboard.jpg) · [Windows build and validation](VALIDATION.md)

Windows graphics and physical sound output still need a playtest on the target machine.
