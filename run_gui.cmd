@echo off
setlocal

set CC=tcc

reg Query "HKLM\Hardware\Description\System\CentralProcessor\0" | find /i "x86" > NUL && (set ARCH=x86) || (set ARCH=x64)

set SDL=.\SDL2-2.24.1
set TTF=.\SDL2_ttf-2.20.1

set PATH=%PATH%;%SDL%\lib\%ARCH%
set PATH=%PATH%;%TTF%\lib\%ARCH%

rem %CC% -DSDL_MAIN_HANDLED -I%SDL%\include -I%TTF%\include -L%SDL%\lib\%ARCH% -L%TTF%\lib\%ARCH% -lSDL2 -lSDL2_ttf -run src\gui.c
%CC% -DSDL_MAIN_HANDLED -I%SDL%\include -I%TTF%\include -L%SDL%\lib\%ARCH%  -L%TTF%\lib\%ARCH% -lSDL2 -lSDL2_ttf -run src\gui.c

