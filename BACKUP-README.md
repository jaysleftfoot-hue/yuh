# November 18 native graphics build

Source commit: `d88e88f268c8294a0b3d198cd355f78676b3b959` on `codex/big-league-bowling`.

- `Big-League-Bowling-Native-Windows-x64.zip`: extract fully and run Big League Bowling.exe. Keep assets beside the EXE. Windows 10/11 x64, OpenGL 3.3. Cross-built on Linux; Windows execution is unverified.
- `Big-League-Bowling-source.zip`: all tracked files from the source commit, including native assets, build instructions and actual gameplay screenshots.
- `yuh-native-preservation.bundle`: complete source-branch Git history. Restore with `git clone yuh-native-preservation.bundle recovered-bowling`.
- `SHA256SUMS.txt`: checksums for the three downloads.

Both native test suites passed. Actual graphical ten-frame matches completed in traditional and mascot modes. The graphics pass adds an enclosed mascot course and inflatable tube, denser traditional lane surroundings, revised camera/HUD/materials and scene antialiasing. See native/docs/VALIDATION.md and REFERENCE_REVIEW.md in the source archive.

This is an artifact-only backup branch. The development branch excludes generated binaries. The earlier November 17 backup remains available separately.
