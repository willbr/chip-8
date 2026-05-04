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
| `src/gui.c` | SDL2 + microui debugger with screen, regs, disassembly, memory |
| `src/main.c` | microui demo UI |
| `src/renderer.c` | SDL2 rendering backend for microui |
| `vendors/microui/` | vendor copy of rxi/microui |

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
* [ ] MCP server
* [ ] Move emulation to a separate library for hot reloading
* [ ] Hot reload ROM via IPC or TCP

## References

- [Wikipedia — CHIP-8](https://en.wikipedia.org/wiki/CHIP-8)
- [XXIIVV — CHIP-8](https://wiki.xxiivv.com/site/chip8.html)
- [Cowgod's Chip-8 Technical Reference](http://devernay.free.fr/hacks/chip8/C8TECH10.HTM)
- [rxi/microui](https://github.com/rxi/microui)
- [dmatlack/chip8 ROMs](https://github.com/dmatlack/chip8/tree/master/roms/games)
- [loktar00/chip8 ROMs](https://github.com/loktar00/chip8/tree/master/roms)
