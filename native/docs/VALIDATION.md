# Native reconstruction verification — November 17, 2026

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
- Exact gameplay matching to the requested YouTube videos; neither clip has been viewed.
- Finished art quality comparable to the supplied commercial-game references. This remains a procedural prototype.

The source, assets, scripts, tests, art direction and this evidence are preserved in Git. Generated build downloads are preserved separately on the artifact-only `backups/native-2026-11-17` Git branch. GitHub Release metadata access worked, but uploads.github.com rejected file uploads with HTTP 401; the empty draft was removed and the verified Git transport is used for these backup files. Future patches must follow the commit/push/remote-verification workflow in ../../AGENTS.md.
