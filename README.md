# chip-8

A CHIP-8 emulator with a CLI frontend and a GUI debugger. Built with Tiny C Compiler (tcc) + SDL2 on Windows.

## Layout

```
┌─────────────┬────────┐
│   screen    │  regs  │
├─────────────┼────────┤
│    mem      │  dis   │
└─────────────┴────────┘
```

## Quick start

```
make run-cli      # terminal output
make run-gui      # SDL2 debugger
make run-demo     # microui demo (not connected to emulation)
make mcp-server   # MCP server (stdio transport)
```

## Architecture

| File | Purpose |
|------|---------|
| `src/cpu.h` | CPU emulation (header-only: init, cycle, draw, dis, load) |
| `src/cli.c` | Terminal frontend, prints screen buffer as ASCII |
| `src/gui.c` | SDL2 + microui debugger with screen, regs, disassembly, memory |
| `src/mcp_cpu.c` | Headless CHIP-8 emulator for MCP server |
| `mcp_server.py` | Python MCP server (stdio transport) |
| `src/main.c` | microui demo UI |
| `src/renderer.c` | SDL2 rendering backend for microui |
| `vendors/microui/` | vendor copy of rxi/microui |

## Todo

### Core emulation

* [x] SDL
* [x] `SDL_ttf`
* [x] Basic opcode execution (`0x1NNN`, `0x6XNN`, `0x7XNN`, `0xANNN`, `0xDNNN`, `0x00E0`)
* [x] Subroutines (`0x2NNN`, `0x00EE`)
* [x] Skip instructions (`0x3XNN`, `0x4XNN`, `0x5XY0`, `0x9XY0`)
* [x] Arithmetic / logic (`0x8XY*`)
* [x] Random (`0xCXNN`)
* [x] Input (`0xEXXX`, `0xFX0A`)
* [x] Delay timer
* [x] Sound timer
* [x] Font data loaded into memory
* [x] Hex keyboard handling
* [x] Test rom https://github.com/corax89/chip8-test-rom
* [ ] Quirk flags (COSMAC VIP vs SCHIP behaviour for `8XY6`/`8XYE`, `FX55`/`FX65`)
* [ ] SCHIP / Super-CHIP support (128x64 hi-res, extended opcodes `00FD`, `00FE`, `FX75`, `FX85`)

### Frontends

* [x] CLI ASCII output
* [x] SDL2 debugger GUI
* [x] MCP server
* [ ] Interactive CLI (REPL: `s`tep, `r`un, `d`ump, `b`reak)
* [ ] Sound beep in CLI

### Debugger / GUI improvements

* [x] Screen rendering
* [x] Register window
* [x] Disassembly window
* [x] Memory window
* [x] Stack window
* [x] On-screen keypad
* [x] ROM browser
* [ ] **Breakpoints** (pause at address)
* [ ] **Scrollable memory viewer** (full 0x000-0xFFF hex dump, not just 128 bytes at `I`)
* [ ] **Scrollable disassembly with PC highlight**
* [ ] **Configurable clock speed** (slider instead of hard-coded 1000 Hz)
* [ ] **Instruction trace log** (ring buffer of last N instructions with register values)
* [ ] **Save / load CPU state** (snapshot serialization)

### MCP server

* [x] Basic commands (INIT, LOAD, STEP, RUN, REGS, MEM, SCREEN, DIS, STATE)
* [ ] `KEY` command (inject keypresses so interactive ROMs work)
* [ ] `TICK` command (decrement delay/sound timers independently of cycles)
* [ ] `BREAKPOINT` commands (set/clear/list)

### Tooling

* [ ] Assembler & Linker
* [ ] REPL assembler inside the debugger
* [ ] Basic pixel / sprite editor in the debugger
* [ ] pico8 lua syntax language that compiles to assembly

## Known issues

* `src/cpu.h`
  * `load()` leaks `FILE*` (no `fclose`)
  * `font_data` and `rng_seeded` lack `static` (ODR risk if header included in multiple TUs)
  * Missing `#include <stdio.h>` and include guard
  * `dis()` declares variables after statements inside `case 0xd000:` without braces (C99 portability)
  * `dis()` format string typo: `%0x` should be `%03x`
  * `load()` ignores `fread` return value
* `src/gui.c`
  * `keys_window()` clears `mouse_key` unconditionally every frame, causing held-mouse flicker
  * `render_screen()` writes RGBA sequentially to `SDL_PIXELFORMAT_RGBA8888`; on little-endian this is interpreted as ABGR. Using `SDL_PIXELFORMAT_ABGR8888` would match the write order.
* `src/mcp_cpu.c`
  * `RUN` / `STEP` never tick timers, so delay/sound timers don't count down via MCP
  * No key-input protocol, so interactive ROMs are untestable through MCP
  * `load()` calls `die()` on failure which kills the process instead of returning JSON error

## References

- [Wikipedia — CHIP-8](https://en.wikipedia.org/wiki/CHIP-8)
- [XXIIVV — CHIP-8](https://wiki.xxiivv.com/site/chip8.html)
- [Cowgod's Chip-8 Technical Reference](http://devernay.free.fr/hacks/chip8/C8TECH10.HTM)
- [rxi/microui](https://github.com/rxi/microui)
- [dmatlack/chip8 ROMs](https://github.com/dmatlack/chip8/tree/master/roms/games)
- [loktar00/chip8 ROMs](https://github.com/loktar00/chip8/tree/master/roms)
