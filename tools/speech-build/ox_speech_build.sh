#!/bin/bash
# OpenX6X00 + speech, built in its own area: ~/openx6x00 (buildroot tree) + /mnt/c/claude/openx6x00 (GUI)
# Since upstream took the speech in (RHVoice with its four free English voices, XARA built from NR5P's source by
# tools/xara/build.sh), RF.Guru's buildroot/local-build.sh is run as it is, outside its Docker container, with the
# two trees named here. The update bundle is signed with ~/.config/openx6x00/update-key.pem (never in a repository)
set -o pipefail
cd ~/openx6x00
export GUI_DIR=/mnt/c/claude/openx6x00
export BR_DIR=$HOME/openx6x00/AetherX6100Buildroot
export MODEL=x6100
[ -f ~/.config/openx6x00/update-key.pem ] || { echo "no signing key at ~/.config/openx6x00/update-key.pem"; exit 1; }
grep -n "define VERSION" $GUI_DIR/src/main.h
[ -n "$DRY" ] && exit 0
bash $GUI_DIR/buildroot/local-build.sh
echo "BUILD_EXIT=$?"
