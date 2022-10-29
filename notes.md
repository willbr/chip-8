notes from:
* https://en.wikipedia.org/wiki/CHIP-8

# Reference

tinyc.games tcc + SDL

https://en.wikipedia.org/wiki/CHIP-8
https://wiki.xxiivv.com/site/chip8.html
http://devernay.free.fr/hacks/chip8/C8TECH10.HTM

Use SDL and microui
https://github.com/rxi/microui

# move emulation to library for hot reloading
# allow hot reloading of rom via ipc or tcp?

# Roms
https://github.com/dmatlack/chip8/tree/master/roms/games
https://github.com/loktar00/chip8/tree/master/roms

# Memory

memory 0x1000, 4096

0x000   0 chip-8 interpreter or font data
0x200 512 most programs start here

0xea0 000 96 bytes used for call stack, internal use and other variables
0xf00 000 display refresh

# Registers
16 8-bit registers v0 to vf
vf doubles as the flag register
vf is the carry flag, and other stuff


# Timers

60Hz
Delay timer
Sound timer

# Input
hex keyboard
16 keys
0 to f

# Graphics
64x32 pixels
monochrome

sprites 8x(1 to 16)
sprites are xor'd
vf is set or cleared, and used for collision detection

# Opcodes
35 codes
two bytes wide
big endian

