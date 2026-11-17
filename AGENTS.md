# Working agreement

The user explicitly asked to preserve everything after patches. At completed, tested milestones, commit the source, assets, build scripts, tests and documentation to the working branch and push it to origin. Verify that `git ls-remote origin refs/heads/<branch>` matches local HEAD. Do not describe work as preserved remotely until that check passes. Do not force-push or overwrite unrelated work.

Cloud environments may reset. Do not rely on local uncommitted files, ignored artifacts, environment drafts, or a published snapshot as the only backup. Keep reproducible build instructions in Git. Keep binaries/dependency caches out of source history; save useful preview images under native/docs and provide a source bundle if pushing is blocked.

The native game is in native/. Graphics is the user's top priority. Read native/ART_DIRECTION.md and native/README.md. Use the existing checkout; do not create a worktree unless explicitly requested. Preserve the earlier browser prototype.
