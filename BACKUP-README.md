# Native bowling preservation snapshot

Source commit: 4cc39e9eef52f7c85477e3b20410f148f54c6be0 on codex/big-league-bowling.

This separate artifact-only branch preserves the Windows ZIP, source ZIP and complete Git bundle after a cloud refresh removed the prior local work. The native implementation was reconstructed and tested; it is not a byte-identical recovery. Download the ZIP files using GitHub's Download raw file action. Verify their hashes against SHA256SUMS.txt. The Windows EXE is cross-built and unsigned; running it on Windows remains unverified. Extract all files and keep assets/ alongside the EXE.

GitHub Release file uploads were denied with HTTP 401, so these files use the working Git transport. Keep this snapshot branch as a backup; continue development on codex/big-league-bowling.
