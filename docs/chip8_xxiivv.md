# XXIIVV — CHIP-8

> Source: https://wiki.xxiivv.com/site/chip8.html

CHIP-8 was created by RCA engineer Joe Weisbecker in 1977 for the COSMAC VIP microcomputer.

The Chip-8 language is capable of accessing up to 4KB (4096 bytes) of RAM, from location 0x000 to 0xFFF (0-4095). The first 512 bytes, from 0x000 to 0x1FF, are where the original interpreter was located, and should not be used by programs.

## Registers

| Register | Size | Description |
|----------|------|-------------|
| V[16] | byte | General purpose |
| I | short | General purpose (address register) |
| PC | short | Program counter |
| SP | byte | Stack pointer |
| DT | byte | Delay timer |
| ST | byte | Sound timer |

## Keypad

The computers which originally used the Chip-8 Language had a 16-key hexadecimal keypad.

```
1  2  3  C
4  5  6  D
7  8  9  E
A  0  B  F
```

## Screen

The original implementation of the Chip-8 language used a 64x32-pixel monochrome display. Programs may also refer to a group of sprites representing the hexadecimal digits 0 through F. These sprites are 5 bytes long, or 8x5 pixels. The data should be stored in the interpreter area of Chip-8 memory (0x000 to 0x1FF).

## Instructions

CHIP-8 instructions are always 2 bytes long and arranged in big-endian order, that is with the most significant byte first. The original implementation of the Chip-8 language includes 36 different instructions, including math, graphics, and flow control functions.

All instructions are 2 bytes long and are stored most-significant-byte first. In memory, the first byte of each instruction should be located at an even addresses. If a program includes sprite data, it should be padded so any instructions following it will be properly situated in RAM.

- **nnn or addr** — A 12-bit value, the lowest 12 bits of the instruction
- **n or nibble** — A 4-bit value, the lowest 4 bits of the instruction
- **x** — A 4-bit value, the lower 4 bits of the high byte of the instruction
- **y** — A 4-bit value, the upper 4 bits of the low byte of the instruction
- **kk or byte** — An 8-bit value, the lowest 8 bits of the instruction

| Opcode | Type | C Pseudo | Explanation |
|--------|------|----------|-------------|
| 0NNN | Call | Calls machine code routine (RCA 1802 for COSMAC VIP) at address NNN. | Not necessary for most ROMs. |
| 00E0 | Display | `disp_clear()` | Clears the screen. |
| 00EE | Flow | `return;` | Returns from a subroutine. |
| 1NNN | | `goto NNN;` | Jumps to address NNN. |
| 2NNN | | `*(0xNNN)()` | Calls subroutine at NNN. |
| 3XNN | Cond | `if (Vx == NN)` | Skips the next instruction if VX equals NN. |
| 4XNN | | `if (Vx != NN)` | Skips the next instruction if VX does not equal NN. |
| 5XY0 | | `if (Vx == Vy)` | Skips the next instruction if VX equals VY. |
| 6XNN | Const | `Vx = N` | Sets VX to NN. |
| 7XNN | | `Vx += N` | Adds NN to VX. (Carry flag is not changed). |
| 8XY0 | Assig | `Vx = Vy` | Sets VX to the value of VY. |
| 8XY1 | BitOp | `Vx |= Vy` | Sets VX to VX or VY. (Bitwise OR operation). |
| 8XY2 | | `Vx &= Vy` | Sets VX to VX and VY. (Bitwise AND operation). |
| 8XY3 | | `Vx ^= Vy` | Sets VX to VX xor VY. |
| 8XY4 | Math | `Vx += Vy` | Adds VY to VX. VF is set to 1 when there's a carry, and to 0 when there is not. |
| 8XY5 | | `Vx -= Vy` | VY is subtracted from VX. VF is set to 0 when there's a borrow, and 1 when there is not. |
| 8XY6 | BitOp | `Vx >>= 1` | Stores the least significant bit of VX in VF and then shifts VX to the right by 1. |
| 8XY7 | Math | `Vx = Vy - Vx` | Sets VX to VY minus VX. VF is set to 0 when there's a borrow, and 1 when there is not. |
| 8XYE | BitOp | `Vx <<= 1` | Stores the most significant bit of VX in VF and then shifts VX to the left by 1. |
| 9XY0 | Cond | `if (Vx != Vy)` | Skips the next instruction if VX does not equal VY. |
| ANNN | MEM | `I = NNN` | Sets I to the address NNN. |
| BNNN | Flow | `PC = V0 + NNN` | Jumps to the address NNN plus V0. |
| CXNN | Rand | `Vx = rand() & NN` | Sets VX to the result of a bitwise and operation on a random number (Typically: 0 to 255) and NN. |
| DXYN | Disp | `draw(Vx, Vy, N)` | Draws a sprite at coordinate (VX, VY) that has a width of 8 pixels and a height of N pixels. Each row of 8 pixels is read as bit-coded starting from memory location I; I value does not change after the execution of this instruction. VF is set to 1 if any screen pixels are flipped from set to unset when the sprite is drawn, and to 0 if that does not happen. |
| EX9E | KeyOp | `if (key() == Vx)` | Skips the next instruction if the key stored in VX is pressed. |
| EXA1 | | `if (key() != Vx)` | Skips the next instruction if the key stored in VX is not pressed. |
| FX07 | Timer | `Vx = get_delay()` | Sets VX to the value of the delay timer. |
| FX0A | KeyOp | `Vx = get_key()` | A key press is awaited, and then stored in VX. (Blocking Operation. All instruction halted until next key event). |
| FX15 | Timer | `delay_timer(Vx)` | Sets the delay timer to VX. |
| FX18 | Sound | `sound_timer(Vx)` | Sets the sound timer to VX. |
| FX1E | MEM | `I += Vx` | Adds VX to I. VF is not affected. |
| FX29 | | `I = sprite_addr[Vx]` | Sets I to the location of the sprite for the character in VX. Characters 0-F (in hexadecimal) are represented by a 4x5 font. |
| FX33 | BCD | `set_BCD(Vx); *(I+0)=BCD(3); *(I+1)=BCD(2); *(I+2)=BCD(1);` | Stores the binary-coded decimal representation of VX, with the most significant of three digits at the address in I, the middle digit at I plus 1, and the least significant digit at I plus 2. |
| FX55 | MEM | `reg_dump(Vx, &I)` | Stores from V0 to VX (including VX) in memory, starting at address I. The offset from I is increased by 1 for each value written, but I itself is left unmodified. |
| FX65 | | `reg_load(Vx, &I)` | Fills from V0 to VX (including VX) with values from memory, starting at address I. The offset from I is increased by 1 for each value written, but I itself is left unmodified. |

## Links

- [Technical Reference (Cowgod's)](http://devernay.free.fr/hacks/chip8/C8TECH10.HTM)
- [Chip-8 Roms](https://github.com/dmatlack/chip8/tree/master/roms/games)
- [Implementation Tutorial](https://tobiasvl.github.io/blog/write-a-chip-8-emulator/)
- [Test ROM](https://github.com/corax89/chip8-test-rom)
- [Uxntal Implementation](https://git.sr.ht/~rabbits/chip8uxn)
- [C Implementation](https://github.com/dmatlack/chip8/blob/main/chip8.c)
- [JS Implementation](https://www.freecodecamp.org/news/creating-your-very-own-chip-8-emulator/)
