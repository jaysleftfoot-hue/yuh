# Windows installer

Build the Windows game first using native/build.py. Then run `makensis native/installer/setup.nsi` with NSIS 3.11 or newer. NSIS resolves File paths relative to the script directory. Output: dist/Big-League-Bowling-Setup.exe.

The installer includes all assets, installs for the current user (no administrator prompt), creates desktop/Start menu shortcuts, and registers an uninstaller in Windows Installed Apps. Finish can launch the game. Career saves are outside the installation directory and survive uninstall. The build is unsigned; Windows may show an unknown-publisher prompt. Installer compilation on Linux is not an install/play/uninstall test on Windows.
