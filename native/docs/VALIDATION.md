# Mascot Ride QA and graphics handoff

- Linux rules and campaign suites passed with AddressSanitizer and UndefinedBehaviorSanitizer. Tests cover a spare after a miss on a full rack, the tenth-frame `X - /` case, and a bonus that changes the match winner while preserving the bowling score.
- X11 input flows passed for the low grip, Straight Ride selection and sling, Stadium keyboard phases and pause, and Dugout Equipment. The low grip went from 45% to full power in the same drag from Y=750 to Y=798.
- Complete native Stadium, Straight Ride and Power Ride matches passed. Checked launch and pin impact in both modes, and a board-only export exited after capture. In the Power Ride capture the bowling scores were 187 and 156, the bonuses were 114 and 104, and both the aquarium board and the match result showed 301–260.
- Windows x64 ZIP and NSIS installer were rebuilt. Package CRC and recorded SHA-256 checks passed. Windows installation and runtime remain unverified.
- [Current captures](mascot-qa-preview.jpg) · [Astra graphics priorities and reproduction commands](ASTRA_HANDOFF.md)

# Independent medal drops

- Native rules and campaign passed. Tested 4,000 seeded schedules across Rookie, Pro, All-Star and Hall of Fame, including drought bounds, per-bowler caps, progressively rarer strong medals and independent RNG streams. A spare or bonus attempt preserves the frame’s medal and claim state.
- Hall of Fame was selectable through the native match setup. Full native Hall of Fame matches completed in Stadium and Mascot Ride.
- The Windows x64 archive and installer were rebuilt. The archive passed CRC and its executable and assets matched the build. Windows execution remains unverified.

# Challenges, replays and random pickups

- Native scoring, lane geometry and campaign tests passed. Coverage includes version 1 save migration, individual challenge unlocks, a simulated 7–10 conversion, replay isolation, strike cues, six pickup types, lane placement, one-time pickup credit and opponent trick expiry.
- Complete headless matches ran in Stadium and Mascot. The X11 input flow still requires separate Spot, Curve and Power confirmations. Native captures show the Wrigley replay and board video, Fenway split and Mascot setup and pin impact. Rechecked the distant board and its render target after correcting live texture sampling.
- Windows x64 ZIP and NSIS installer rebuilt. ZIP CRC and byte comparisons of assets and executable passed. Windows runtime and installer execution remain unverified.
- Read ASTRA_HANDOFF.md for the graphics interfaces, reference sources and capture commands.

# Illustrated ball collection

- Verified 64 distinct 768 × 384 textures and compared every packaged asset and the executable against the Windows build. The portable ZIP passed its CRC check.
- Native rules and campaign suites passed. On Linux, opened the locker with an unlocked test profile, rotated Tropicana Finale, equipped it and launched it through Spot, Curve and Power.
- Collection sheet, locker and ready-to-bowl views are in ball-collection.jpg, collection-locker.jpg and collection-lane.jpg. The Windows x64 executable and NSIS installer were cross-built. Windows playback and installation still need a Windows test.

# Live pins and rival difficulty

- Native rules and campaign checks passed. The rack tests cover a live fall, the same pin IDs on a spare attempt and the rival's independent rack.
- Full deterministic matches scored 113 / 182 / 235 for Rookie / Pro / All-Star in Stadium and 115 / 193 / 202 in Mascot. Native window smoke runs completed in Rookie Stadium and All-Star Mascot.
- X11 input confirmed the setup selector carried All-Star into the match and the three stadium shot confirmations still worked. The impact capture shows four standing pins on the HUD and Wrigley scoreboard before the roll is scored.
- Windows x64 ZIP and installer rebuilt. ZIP CRC and packaged asset equality passed. Windows execution remains unverified.

# Painted arcade UI and three-phase stadium shots

- Linux build and native rules/campaign checks passed, including independent Spot/Curve/Power transitions, locked fields, back/reset behavior, lane projection/geometry and launch timing regressions.
- Real native X11 mouse input: first confirmation entered Curve without launching; second entered Power without launching; third launched. Position 4.595 and curve 0.4 remained unchanged through power. Keyboard Space confirmations passed; entering gameplay or resuming via Enter does not accidentally advance a phase. Backspace returned to Curve before a later launch.
- Real menu flows checked: title to setup, locked park prevented start, mascot entry, ball locker including final locked ball, help, scores and pause. Visual captures: arcade-setup.png, arcade-locker.png, arcade-help.png, arcade-scores.png, arcade-pause.png, arcade-power.png, arcade-curve.png and arcade-results.png. Reproduce input checks with `python3 native/tests/ui_flow.py phases` or `keyboard` after building Linux; other documented scenarios capture menu screens.
- Complete traditional AI smoke: 38 rolls, player 63 / rival 122. Complete mascot AI smoke: 41 rolls, player 93 / rival 138. Screenshot/smoke modes do not save progression.
- Visually checked launch and pin-impact scenes in both modes. Canonical geometry/camera/speed files unchanged. Actual ray 3D mesh retained. Locker uses sphere-shaded skins instead of rectangular swatches; UI shader compiled and rendered on Linux.
- Windows x64 cross-build and per-user NSIS installer rebuilt. ZIP CRC/asset equality verified. Installer/ZIP hashes in WINDOWS_BUILD.sha256. Windows execution, installation and GPU appearance remain unverified on an actual Windows machine.
- Internet references were fetched from the official Steam screenshot catalog, and setup/overlay screenshots inspected. URLs/provenance in ../assets/ARCADE_UI.md. New frame/slate/button textures are original artwork, not extracted commercial assets.

Earlier milestone checks follow; newer controls/art notes above supersede conflicting history.

# Character artwork and ray model milestone

- Native scoring, geometry, pacing, physics and campaign tests pass. Lane dimensions, cameras, throw speed, physics body and input mappings are unchanged.
- The ray uses a new closed 3D mesh with smooth normals, continuous swept fins and colored upper/lower surfaces. Face, cap, tapered animated tail, banking and tube are rendered in 3D. Sling attachment was lowered to the tube to avoid crossing the fins.
- Launch and pin-impact views inspected in both modes. Umpire and ball-boy stadium previews verify portrait atlas selection; the helmet batter and Rays player also display correctly. Portraits are vector-style raster artwork, not editable SVGs.
- Windows build and NSIS installer compile checked; ZIP CRC and packaged artwork compared to source. Windows runtime remains unverified.

# Illustrated title / pacing / floor shader milestone

- Original illustrated title-screen texture is packaged, with native menu hit targets. Real X11 mouse navigation verified Help → Back, Scores → Back, Locker → Back, Single Player → Setup → Gameplay, and Mascot Adventure → Setup → Gameplay. Preview tracing confirmed screen and mode transitions. F12 captures/exits when using --screenshot for interactive verification.
- Real-time frame accumulation now accepts up to 250ms per frame; physics still advances in 1/120s steps. The old 50ms cap slowed throws on machines rendering below 20 FPS. Long stalls remain capped.
- Physics tests measure full-power travel to the head pin within 3–4 seconds (nominal 3.5) and verify actual rack contact at low, medium and full power. Lane size and camera presets unchanged. Rules and campaign suites pass.
- Full native stadium match completed: 38 rolls, 63–122. Launch/impact screenshots inspected in both modes; floor renders on both sides in Linux.
- Fixed undefined GLSL pow() calls with signed bases in floor reflection and edge highlights. This is a plausible cause of the reported half-black lane on Windows; confirmation on the affected GPU remains outstanding.
- Windows cross-build, ZIP CRC, complete source-to-package asset comparison and warning-free NSIS installer compile passed. Windows runtime is unverified.

# Long-lane arcade frame milestone

- Intentional playable run increase: 80 to 160 units; width, bodies and camera constants unchanged. Projection tests lock distant rack to 38–46px at 1280×800. Physics/campaign tests use geometry-derived time budgets and pass.
- Full native stadium match completed: 38 rolls, 68–124. Launch and impact captures inspected in both modes.
- Original baseball arcade frame, left power strip, portraits and unboxed Bangers menu lettering replace rectangular application-style controls. UI click regions still exclude the side strip from bowling input. Native window resize continues to map coordinates to the 1280×800 reference canvas.
- Windows cross-build and warning-free NSIS installer compilation pass; installer and portable ZIP include Bangers font and OFL license. Windows runtime remains unverified.
- Exact source-game dimensions are unknown. The supplied graveyard screenshot governs visible composition, not a claim of pixel-identical reproduction. Art remains procedural.

# Arcade controls and installer milestone

- Linux scoring, geometry/projection, physics and campaign suites pass. Hit tests include clicking the tube below Y=630 and exclude side HUD; stationary hold power is covered.
- Real X11 mouse input at (640,710) pulled the mascot sling; release reached the pin deck and knocked pins down. Stadium press/upward drag displayed 81% power. Launch and impact captures cover both modes.
- Windows native cross-build and ZIP CRC pass; every packaged asset equals source. NSIS 3.11 installer compiles without warnings. Installer includes user-level shortcuts, launch-on-finish and uninstall registration; no Windows runtime install/uninstall/play verification is available.
- Reviewed supplied video frames and official Steam screenshots (app32160). Matching is visual, not a claim of exact original game dimensions or input semantics. See CONTROL_REFERENCE.md.
- Current screenshot: tropicana-stadium.png; input evidence: alley-power.png, mascot-pull.png and mascot-release.png. The held mascot image predates the final circular power-icon styling.

# Stadium identity and camera follow-up

- World lane remains 80×10; ball, pin, rack and simulation dimensions unchanged. Stadium ready camera deliberately raised and pulled back. Full camera constants and independent perspective projection now have regression checks (about 295 pixels foul-line-to-head-pin base at 1280×800).
- Linux scoring/geometry/physics and campaign suites pass. Real native launch/impact captures checked for both modes; Wrigley, Chase and Tropicana stadium previews regenerated.
- Actual stadium wordmarks for all ten parks and MLB team insignia load from packaged PNGs. Sources recorded in assets/parks/SOURCES.md. Reference photographs were inspected as a contact sheet; photographs are not bundled.
- Video displays show live bowling scores over layered team graphics. Tropicana gets a wide video wall, roof catwalk and orange landmark. No claim of photoreal stadium reconstruction.
- Tropicana full graphical match completed: 37 rolls, 69–200; live scoreboard result captured.
- Windows cross-build succeeds; all 20 logo PNGs and source attribution must match the ZIP. Windows runtime remains unverified.

# Current stadium / Rays / scoreboard validation

- Linux build passes scoring, geometry regression, physics and campaign checks, including 64 designs, ten stadiums, save/restore, powers, steering, boost and AI matches.
- Full native matches complete: stadium 38 rolls, 56–119; mascot 41 rolls, 87–127.
- Playable length deliberately increased from 42 to 80 units; width remains 10. Shared geometry, camera and rack-fit regression checks prevent scenery edits from independently changing proportions.
- Launch and pin-deck captures cover both modes. Wrigley manual scoreboard texture and in-world board visually checked; live bowling values replace static title signs.
- Real X11 mouse press/backward-sideways drag and release exercised mascot sling controls.
- Windows cross-build and ZIP verification recorded in WINDOWS_BUILD.sha256. Windows execution and audio playback remain unverified.
- Visuals remain procedural, with authored art still needed to approach the commercial references. Both supplied Dropbox clips were sampled; full YouTube videos remain unviewed. Button semantics are inferred from footage.

Earlier validation history follows; its dimensions and screenshots may describe superseded builds.

# Native verification — November 18, 2026

## Graphics pass (November 18)

- Linux release build passed all nine rules/physics scenarios and the campaign suite.
- Real native full-match smoke checks passed for traditional mode (37 rolls, 115–119) and mascot mode (41 rolls, 72–35). Scoring, AI, progression and powers remain in the existing implementation.
- Lighting and scene-only FXAA shaders compiled successfully. No graphical warnings/errors were reported; the only runtime warning was the unavailable audio playback device.
- Actual game captures were visually inspected at launch in both modes and during a mascot roll. The camera keeps the ring above the controls. A separate clubhouse camera stays inside the new mascot tunnel.
- `tropicana-mascot.png`, `wrigley-traditional.png` and `tropicana-in-motion.png` show this graphics pass; REFERENCE_REVIEW.md gives reproducible capture commands and honestly scopes the inspected reference.
- Windows x64 cross-build succeeded. Both shaders and the font/license are included in the ZIP; CRC and source-to-package equality were checked. WINDOWS_BUILD.sha256 identifies the updated package. Windows runtime execution remains unverified.

## Earlier reconstruction (November 17)

The native files were reconstructed from the conversation after a cloud refresh removed uncommitted workspace changes. This is not a byte-for-byte recovery of those missing files.

## Verified

- `python3 native/build.py --sdk-cache /workspace/tools` built the Linux game and passed all nine rules/physics scenarios plus the campaign suite (64 unique design names, ten ordered stadiums, save/restore/corrupt-save handling, powers, steering, boost and two-player matches).
- CMake configured and built with the official raylib 5.5 SDK. CTest completed 2/2 test targets with zero failures.
- Real native graphical smoke runs completed both competitors' ten-frame games:
  - Traditional: 37 rolls, player 115, rival 119.
  - Mascot: 41 rolls, player 72, rival 35.
- GLSL shaders compiled without warnings/errors. The Tropicana mascot screenshot in this folder was captured from the new running build and visually inspected.
- Windows x64 cross-build succeeded using Zig 0.14.1 and raylib 5.5. `file` identifies a PE32+ x86-64 Windows GUI executable. ZIP CRC verification passed, and the packaged EXE and shader matched the generated files. The ZIP includes all runtime assets and licenses. WINDOWS_BUILD.sha256 identifies that build.
- Source/assets were pushed to `codex/big-league-bowling`. The initial reconstructed source commit `c59d82b05d5a24f22d43e1aa0dfecef8d1acae43` matched GitHub's remote HEAD.
- A fresh clone from GitHub in `/tmp/yuh-preservation-check` at that commit rebuilt successfully and reran both native test suites. This verifies source recovery from GitHub rather than dependence on uncommitted workspace files.

## Still unverified / incomplete

- Running the EXE on Windows; the Linux cross-build is not a Windows runtime test.
- Audio playback on this machine (no playback device).
- Exact gameplay matching to the requested YouTube videos. A 20.733-second Polar Bowler Dropbox clip was sampled visually on November 18; the second video and full YouTube clips remain unviewed.
- Finished art quality comparable to the supplied commercial-game references. This remains a procedural prototype.

The source, assets, scripts, tests, art direction and this evidence are preserved in Git. Generated build downloads are preserved separately on the artifact-only `backups/native-2026-11-17` Git branch. GitHub Release metadata access worked, but uploads.github.com rejected file uploads with HTTP 401; the empty draft was removed and the verified Git transport is used for these backup files. The November 18 graphics build has its own artifact backup on `backups/native-graphics-2026-11-18`. Future patches must follow the commit/push/remote-verification workflow in ../../AGENTS.md.

## Lane/HUD follow-up

Linux build and rules/campaign suites pass. Full graphical matches completed: traditional 37 rolls (115–119), mascot 41 rolls (72–35). Launch screenshots were regenerated after the camera framing correction. Windows was cross-built again; execution remains unverified. The `tropicana-in-motion.png` image above belongs to the earlier graphics pass; the two launch screenshots show the current lane/HUD.

## Zombies controls and alley correction

The 26.333-second Zombies clip is now accessible and was visually sampled, including 2 fps through the control sequence. Traditional's stretched hall was replaced with a compressed arcade presentation. The on-lane marker/power meter follows the observed visual stages; exact button semantics remain inferred.

Linux build passed all nine physics/rules scenarios and the campaign suite, including new drag power/aim, locked position, bounds and downward-drag checks. Both graphical matches passed (traditional 37 rolls, 115–119; mascot 41 rolls, 72–35). Real X11 mouse movement/press/drag was injected into the native window and the 81% held-power screenshot was visually checked. Windows cross-build and ZIP verification passed; Windows execution remains unverified.

A second real X11 input run pressed, dragged and released the mouse. The captured result showed the ball at the pin deck, fallen pins and the following camera, confirming release launches the shot. The original game's exact mouse-button semantics still cannot be proven from a screen recording alone.

## Slim scorecard / neon wall pass

Linux build and existing rules/campaign suites passed. A full traditional match completed in 37 rolls (115–119). Visually inspected both the initial scene and completed scorecards, including strikes, spares, cumulative totals and tenth-frame bonus marks. Saved screenshots: wrigley-traditional.png and scorecard-result.png. No graphical shader errors. Windows cross-build completed; Windows execution remains unverified.
