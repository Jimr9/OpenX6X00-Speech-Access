# Building OpenX6X00 Speech Access releases on a PC

Releases are built on a PC (WSL2, Ubuntu 22.04) and published on GitHub; radios update from the GitHub release
(Settings, Check, Install).

- `ox_speech_build.sh`: the full build. It makes a copy of RF.Guru's `buildroot/local-build.sh` with three changes
  (the XARA step skipped: its source is gone from GitHub; RHVoice and six English voices kept; the GUI package
  depending on rhvoice) and runs it. Expects the buildroot tree at `~/openx6x00/AetherX6100Buildroot`
  (`git clone --recurse-submodules https://github.com/Guru-RF/AetherX6100Buildroot.git`; the 0.3.12.1 release was
  built on its commit 273083c; local-build.sh changes that tree in place) and this repository at
  `/mnt/c/claude/openx6x00`.
- It signs the update bundle with `~/.config/openx6x00/update-key.pem`. **The key is in no repository**: keep backups
  of it (a copy is at `C:\Users\Jim\.config\openx6x00-speech\update-key.pem`). Its public half is
  `rootfs/usr/share/x6100/update.pub`. The same key signs X6100 Accessibility.
- `verify.sh`: checks the speech strings in the built GUI and zips the image (adjust the test name inside).

PC setup (WSL, packages, cmake 3.28.3, `/etc/wsl.conf` without the Windows PATH): see
https://github.com/Jimr9/X6100-Accessibility-Testing/tree/main/tools/local-build

Making a release:

1. Raise the version in `src/main.h` (`OpenX6X00 - v0.3.12.2`).
2. `bash tools/speech-build/ox_speech_build.sh` in WSL (ends with `BUILD_EXIT=0`).
3. From `build/images/`: the card image (`OpenX6X00-x6100-local-sdcard.img`, zipped as
   `OpenX6X00-Speech-Access-x6100-v<version>-sdcard.img.zip`) and `OpenX6X00-x6100-v<version>-update.tar.gz`.
4. Commit, push, and `gh release create v<version> --latest` with both files. The radios read the latest release, so
   it must not be a pre-release.
