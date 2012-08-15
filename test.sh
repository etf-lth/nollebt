#!/bin/sh
echo
echo "#####################"
echo "# building backpack"
echo "#####################"
echo

cd backpack
make
cd ..

echo
echo "#####################"
echo "# patching firmware"
echo "#####################"
echo

cd patch
./patch_rok101008_backpack.py
cd ..

echo
echo "#####################"
echo "# flashing module"
echo "#####################"
echo

./rom.py
