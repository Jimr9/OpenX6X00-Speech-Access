#!/bin/bash
# OpenX6X00 + speech, built in its own area: ~/openx6x00 (buildroot tree) + /mnt/c/claude/openx6x00 (GUI, branch speech)
# Our copy of local-build.sh, changed in three ways:
#  - XARA skipped (its source is gone from GitHub; the GUI's CMake treats it as optional)
#  - RHVoice kept, with the six English voices of the X6100 Accessibility fork
#  - the GUI package depends on rhvoice
set -o pipefail
cd ~/openx6x00
export GUI_DIR=/mnt/c/claude/openx6x00
export BR_DIR=$HOME/openx6x00/AetherX6100Buildroot
export MODEL=x6100
S=~/openx6x00/local-build-speech.sh

sed -e '/^echo "== XARA/,/^grep -q .*-DXARA_BINARY/{/make toolchain/!d}' \
    -e '/^echo "== No RHVoice/,/^! grep -in "rhvoice"/d' \
    -e '/^rm -rf \$BR\/build\/target\/usr\/share\/RHVoice/d' \
    $GUI_DIR/buildroot/local-build.sh > $S
sed -i 's|^(cd \$BR/build && make toolchain)$|XARA_CC=$BR/build/host/bin/arm-buildroot-linux-gnueabihf-gcc\n&|' $S

# The speech block, before the system cleanup
cat > ~/openx6x00/speech-block.txt <<'EOF'
echo "== Speech: RHVoice and six English voices (X6100 Accessibility fork)"
for o in RHVOICE RHVOICE_ENGLISH RHVOICE_SLT RHVOICE_ALAN RHVOICE_BDL RHVOICE_CLB RHVOICE_ENGENIY_EN RHVOICE_LYUBOV; do
    grep -q "^BR2_PACKAGE_$o=y" $DEFCONFIG || echo "BR2_PACKAGE_$o=y" >> $DEFCONFIG
done
grep -q "rhvoice" $GUI_MK || sed -i "/^${GUI_VAR}_DEPENDENCIES/s/$/ rhvoice/" $GUI_MK
grep "^${GUI_VAR}_DEPENDENCIES" $GUI_MK
grep "^BR2_PACKAGE_RHVOICE" $DEFCONFIG

EOF
awk -v blk=$HOME/openx6x00/speech-block.txt '
    /^echo "== System cleanup/ { while ((getline l < blk) > 0) print l }
    { print }' $S > $S.new && mv $S.new $S
echo "---- script check"
grep -n "XARA\|RHVoice\|rhvoice\|== Speech\|== System cleanup" $S
echo "----"
[ -n "$DRY" ] && exit 0; bash $S
echo "BUILD_EXIT=$?"
