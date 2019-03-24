#!/bin/csh
cd Release

7z A -t7z bvox_win32_311.7z \
	InstMsi.Exe InstMsiW.Exe Setup.Exe Setup.ini brainvox_win32.msi

cat ../7zS.sfx ../config.txt bvox_win32_311.7z > ../bvox_win32_311.exe
chmod +x ../bvox_win32_311.exe

rm bvox_win32_311.7z 
