# Illustrated title and faster throws

Source 1c1447e40c66bfffd9b78900f682aa5abd5a88d2 on codex/big-league-bowling. Original painted baseball title screen, working native menus, ~3.5s full-power throws with lane size unchanged, and GLSL floor fix for signed pow operations. Linux tests, menu navigation and full match passed. Installer/ZIP compiled and checked; Windows runtime and affected-GPU confirmation remain unverified. Run Big-League-Bowling-Setup.exe on Windows x64; unsigned. Verify SHA256SUMS.txt.

Includes real-time accumulation fix for rendering below 20 FPS; fixed 120Hz physics steps retained.
