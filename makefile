watch-cli:
	watchexec -cr -f "*.c" -f "*.h" "make run-cli"

run-cli:
	tcc -run .\src\cli.c

watch-gui:
	watchexec -cr -f "*.c" -f "*.h" "make run-gui"

run-gui:
	run_gui.cmd

watch-demo:
	watchexec -cr -f "*.c" -f "*.h" "make run-demo"

run-demo:
	run_demo.cmd

