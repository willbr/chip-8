watch:
	watchexec -cr -f "*.c" -f "*.h" "make run"

run:
	tcc -run .\src\main.c

