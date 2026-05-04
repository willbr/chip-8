@echo off
setlocal

if "%1"=="watch-cli" goto watch-cli
if "%1"=="run-cli" goto run-cli
if "%1"=="watch-gui" goto watch-gui
if "%1"=="run-gui" goto run-gui
if "%1"=="watch-demo" goto watch-demo
if "%1"=="run-demo" goto run-demo
if "%1"=="hydrate-roms" goto hydrate-roms
if "%1"=="mcp-server" goto mcp-server

echo Usage: make [watch-cli ^| run-cli ^| watch-gui ^| run-gui ^| watch-demo ^| run-demo ^| hydrate-roms ^| mcp-server]
exit /b 1

:watch-cli
watchexec -cr -f "*.c" -f "*.h" "make run-cli"
exit /b 0

:run-cli
tcc -run .\src\cli.c
exit /b 0

:watch-gui
watchexec -cr -f "*.c" -f "*.h" "make run-gui"
exit /b 0

:run-gui
call run_gui.cmd
exit /b 0

:watch-demo
watchexec -cr -f "*.c" -f "*.h" "make run-demo"
exit /b 0

:run-demo
call run_demo.cmd
exit /b 0

:hydrate-roms
echo Hydrating ROMs from dmatlack/chip8, loktar00/chip8, and corax89/chip8-test-rom...
if not exist roms\dmatlack mkdir roms\dmatlack
if not exist roms\loktar00 mkdir roms\loktar00
if not exist roms\corax89 mkdir roms\corax89

git clone --depth 1 https://github.com/dmatlack/chip8.git %TEMP%\chip8-dmatlack >nul 2>&1
xcopy /Y /S %TEMP%\chip8-dmatlack\roms\games\*.ch8 roms\dmatlack\ >nul 2>&1
rmdir /S /Q %TEMP%\chip8-dmatlack >nul 2>&1

git clone --depth 1 https://github.com/loktar00/chip8.git %TEMP%\chip8-loktar00 >nul 2>&1
xcopy /Y /S %TEMP%\chip8-loktar00\roms\*.ch8 roms\loktar00\ >nul 2>&1
rmdir /S /Q %TEMP%\chip8-loktar00 >nul 2>&1

git clone --depth 1 https://github.com/corax89/chip8-test-rom.git %TEMP%\chip8-corax89 >nul 2>&1
xcopy /Y /S %TEMP%\chip8-corax89\*.ch8 roms\corax89\ >nul 2>&1
rmdir /S /Q %TEMP%\chip8-corax89 >nul 2>&1

echo Done.
exit /b 0

:mcp-server
if not exist bin\mcp_cpu.exe (
    echo Building mcp_cpu.exe...
    if not exist bin mkdir bin
    tcc src\mcp_cpu.c -o bin\mcp_cpu.exe
)
python mcp_server.py
exit /b 0
