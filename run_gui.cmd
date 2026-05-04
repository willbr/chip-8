@echo off
setlocal

set CC=tcc

reg Query "HKLM\Hardware\Description\System\CentralProcessor\0" | find /i "x86" > NUL && (set ARCH=x86) || (set ARCH=x64)

set SDL=.\vendors\SDL2-2.24.1
set TTF=.\vendors\SDL2_ttf-2.20.1

set PATH=%PATH%;%SDL%\lib\%ARCH%
set PATH=%PATH%;%TTF%\lib\%ARCH%

%CC% -DSDL_MAIN_HANDLED -I%SDL%\include -Ivendors\microui\src -L%SDL%\lib\%ARCH% -lSDL2 vendors\microui\src\microui.c src\renderer.c -run src\gui.c

