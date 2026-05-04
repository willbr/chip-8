@echo off
setlocal

if "%1"=="watch-cli" goto watch-cli
if "%1"=="run-cli" goto run-cli
if "%1"=="watch-gui" goto watch-gui
if "%1"=="run-gui" goto run-gui
if "%1"=="watch-demo" goto watch-demo
if "%1"=="run-demo" goto run-demo

echo Usage: make [watch-cli ^| run-cli ^| watch-gui ^| run-gui ^| watch-demo ^| run-demo]
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
