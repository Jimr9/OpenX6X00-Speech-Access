#!/bin/bash
IMG=~/openx6x00/AetherX6100Buildroot/build/images
T=~/openx6x00/AetherX6100Buildroot/build/target/usr/sbin/x6100_gui
ls -la --time-style=full-iso $T $IMG/OpenX6X00-x6100-local-sdcard.img
for s in "Boot menu key F" "Speech when the screen is off" "Restart into, once" "Talking menu waits" " Speech mode "; do
  printf '%-32s %s\n' "$s" "$(grep -c "$s" $T)"
done
grep -n "warning" ~/openx6x00-test9.log | grep -i "bootmenu\|voice_text" | head
grep -c "x6100_gui" ~/openx6x00-test9.log
cd $IMG && rm -f /mnt/c/Users/Jim/Downloads/OpenX6X00-speech-test9.img.zip && zip -j -q /mnt/c/Users/Jim/Downloads/OpenX6X00-speech-test9.img.zip OpenX6X00-x6100-local-sdcard.img && ls -la /mnt/c/Users/Jim/Downloads/OpenX6X00-speech-test9.img.zip
