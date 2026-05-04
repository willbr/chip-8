# chip-8 emulator

## Commands

| Command | What it does |
|---------|-------------|
| `make run-cli` | Run CLI frontend (tcc -run src/cli.c) |
| `make run-gui` | Run debugger GUI (runs `run_gui.cmd`) |
| `make run-demo` | Run microui demo (runs `run_demo.cmd`) |
| `make mcp-server` | Run MCP server (stdio transport) |
| `make watch-cli` | Auto-rebuild CLI on `.c`/`.h` change (requires watchexec) |

Toolchain: **tcc** (Tiny C Compiler) + **SDL2** 2.24.1 + **SDL_ttf** 2.20.1 on Windows. No Makefile for building — `run_*.cmd` scripts handle compile+run.

## Architecture

- `src/cpu.h` — CPU emulation is **header-only** with inline implementations (init, cycle, draw, dis, load). Include it directly.
- `src/cli.c` — Terminal frontend, prints screen buffer as ASCII.
- `src/gui.c` — SDL2 + microui debugger showing screen, regs, disassembly, memory.
- `src/mcp_cpu.c` — headless CHIP-8 emulator process for the MCP server.
- `mcp_server.py` — Python MCP server (stdio transport) wrapping `mcp_cpu.exe`.
- `src/main.c` + `src/renderer.c` — microui demo UI. **Not connected to CHIP-8 emulation.**
- `vendors/microui/` — vendor copy of rxi/microui.

## Important conventions

- `cpu.h` is a header with function definitions, not a proper `.c`/`.h` split. Do not try to compile it separately.
- `_STDINT_H_` is defined before `#include <SDL.h>` to prevent SDL's stdint shim from conflicting with tcc. Keep this pattern in SDL files.
- Binaries (`*.exe`) in `bin/` and repo root should **not** be committed. `.gitignore` already excludes `*.exe`.
- ROMs load at `0x200`; font data is defined in `cpu.h` but **not loaded into memory**.

## Known incomplete

Most opcodes are stubbed with `die()`. Missing: subroutines (`0x2NNN`, `0x00EE`), skip instructions (`0x3XNN`, `0x4XNN`, `0x5XY0`, `0x9XY0`), arithmetic (`0x8XY*`), random (`0xCXNN`), input (`0xEXXX`, `0xFX0A`), delay/sound timers, hex keyboard handling. See `README.md` todo list.
