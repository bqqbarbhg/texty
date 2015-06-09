@echo off

call "C:\Program Files (x86)\Microsoft Visual Studio 12.0\VC\vcvarsall.bat" x64
cd bin

set IgnoreWarn= -wd4100
set CLFlags= -MT -nologo -Od -W4 -WX -Zi %IgnoreWarn% -D_CRT_SECURE_NO_WARNINGS
set LDFlags= -opt:ref user32.lib gdi32.lib

cl %CLFlags% ../src/build.cpp -link %LDFlags% -subsystem:windows -out:texty.exe
cd ..

