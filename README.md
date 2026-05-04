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
make run-cli   # terminal output
make run-gui   # SDL2 debugger
make run-demo  # microui demo (not connected to emulation)
```

## Architecture

| File | Purpose |
|------|---------|
| `src/cpu.h` | CPU emulation (header-only: init, cycle, draw, dis, load) |
| `src/cli.c` | Terminal frontend, prints screen buffer as ASCII |
| `src/gui.c` | SDL2 + SDL_ttf debugger with screen, regs, disassembly, memory |
| `src/main.c` | microui demo UI |
| `src/renderer.c` | SDL2 rendering backend for microui |
| `microui/` | git submodule (rxi/microui) |

## Todo

* [x] SDL
* [x] `SDL_ttf`
* [x] Basic opcode execution (`0x1NNN`, `0x6XNN`, `0x7XNN`, `0xANNN`, `0xDNNN`, `0x00E0`)
* [ ] Subroutines (`0x2NNN`, `0x00EE`)
* [ ] Skip instructions (`0x3XNN`, `0x4XNN`, `0x5XY0`, `0x9XY0`)
* [ ] Arithmetic / logic (`0x8XY*`)
* [ ] Random (`0xCXNN`)
* [ ] Input (`0xEXXX`, `0xFX0A`)
* [ ] Delay timer
* [ ] Sound timer
* [ ] Font data loaded into memory
* [ ] Hex keyboard handling
* [ ] Test rom https://github.com/corax89/chip8-test-rom
