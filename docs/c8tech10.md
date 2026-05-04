# Cowgod's Chip-8 Technical Reference v1.0

> Source: http://devernay.free.fr/hacks/chip8/C8TECH10.HTM
> Compiled by Thomas P. Greene (cowgod@rockpile.com)
> August 30, 1997

## 1.0 - About Chip-8

Chip-8 is a simple, interpreted, programming language which was first used on some do-it-yourself computer systems in the late 1970s and early 1980s. The COSMAC VIP, DREAM 6800, and ETI 660 computers are a few examples. These computers typically were designed to use a television as a display, had between 1 and 4K of RAM, and used a 16-key hexadecimal keypad for input. The interpreter took up only 512 bytes of memory, and programs, which were entered into the computer in hexadecimal, were even smaller.

In the early 1990s, the Chip-8 language was revived by a man named Andreas Gustafsson. He created a Chip-8 interpreter for the HP48 graphing calculator, called Chip-48. The HP48 was lacking a way to easily make fast games at the time, and Chip-8 was the answer. Chip-48 later begat Super Chip-48, a modification of Chip-48 which allowed higher resolution graphics, as well as other graphical enhancements.

Chip-48 inspired a whole new crop of Chip-8 interpreters for various platforms, including MS-DOS, Windows 3.1, Amiga, HP48, MSX, Adam, and ColecoVision.

## 2.0 - Chip-8 Specifications

### 2.1 - Memory

The Chip-8 language is capable of accessing up to 4KB (4,096 bytes) of RAM, from location 0x000 (0) to 0xFFF (4095). The first 512 bytes, from 0x000 to 0x1FF, are where the original interpreter was located, and should not be used by programs.

Most Chip-8 programs start at location 0x200 (512), but some begin at 0x600 (1536). Programs beginning at 0x600 are intended for the ETI 660 computer.

**Memory Map:**

```
+---------------+ = 0xFFF (4095) End of Chip-8 RAM
|               |
|               |
| 0x200 to 0xFFF|
| Chip-8        |
| Program / Data|
|     Space     |
|               |
+---------------+ = 0x600 (1536) Start of ETI 660 Chip-8 programs
|               |
+---------------+ = 0x200 (512)  Start of most Chip-8 programs
| 0x000 to 0x1FF|
| Reserved for  |
|  interpreter  |
+---------------+ = 0x000 (0)    Start of Chip-8 RAM
```

### 2.2 - Registers

Chip-8 has 16 general purpose 8-bit registers, usually referred to as Vx, where x is a hexadecimal digit (0 through F). There is also a 16-bit register called I. This register is generally used to store memory addresses, so only the lowest (rightmost) 12 bits are usually used.

The VF register should not be used by any program, as it is used as a flag by some instructions.

Chip-8 also has two special purpose 8-bit registers, for the delay and sound timers. When these registers are non-zero, they are automatically decremented at a rate of 60Hz.

There are also some "pseudo-registers" which are not accessible from Chip-8 programs:
- **Program counter (PC)** — 16-bit, stores the currently executing address
- **Stack pointer (SP)** — 8-bit, points to the topmost level of the stack
- **Stack** — array of 16 16-bit values, for up to 16 levels of nested subroutines

### 2.3 - Keyboard

16-key hexadecimal keypad layout:

```
1  2  3  C
4  5  6  D
7  8  9  E
A  0  B  F
```

### 2.4 - Display

The original implementation of the Chip-8 language used a 64x32-pixel monochrome display:

```
(0,0)              (63,0)
+------------------+
|                  |
|                  |
|                  |
|                  |
+------------------+
(0,31)             (63,31)
```

Some other interpreters, most notably the one on the ETI 660, also had 64x48 and 64x64 modes. Super Chip-48 added a 128x64-pixel mode.

Chip-8 draws graphics on screen through the use of sprites. A sprite is a group of bytes which are a binary representation of the desired picture. Chip-8 sprites may be up to 15 bytes, for a possible sprite size of 8x15.

Programs may also refer to a group of sprites representing the hexadecimal digits 0 through F. These sprites are 5 bytes long, or 8x5 pixels. The data should be stored in the interpreter area of Chip-8 memory (0x000 to 0x1FF).

**Hexadecimal font data:**

| Char | Binary | Hex |
|------|--------|-----|
| 0 | 11110000 10010000 10010000 10010000 11110000 | F0 90 90 90 F0 |
| 1 | 00100000 01100000 00100000 00100000 01110000 | 20 60 20 20 70 |
| 2 | 11110000 00010000 11110000 10000000 11110000 | F0 10 F0 80 F0 |
| 3 | 11110000 00010000 11110000 00010000 11110000 | F0 10 F0 10 F0 |
| 4 | 10010000 10010000 11110000 00010000 00010000 | 90 90 F0 10 10 |
| 5 | 11110000 10000000 11110000 00010000 11110000 | F0 80 F0 10 F0 |
| 6 | 11110000 10000000 11110000 10010000 11110000 | F0 80 F0 90 F0 |
| 7 | 11110000 00010000 00100000 01000000 01000000 | F0 10 20 40 40 |
| 8 | 11110000 10010000 11110000 10010000 11110000 | F0 90 F0 90 F0 |
| 9 | 11110000 10010000 11110000 00010000 11110000 | F0 90 F0 10 F0 |
| A | 11110000 10010000 11110000 10010000 10010000 | F0 90 F0 90 90 |
| B | 11100000 10010000 11100000 10010000 11100000 | E0 90 E0 90 E0 |
| C | 11110000 10000000 10000000 10000000 11110000 | F0 80 80 80 F0 |
| D | 11100000 10010000 10010000 10010000 11100000 | E0 90 90 90 E0 |
| E | 11110000 10000000 11110000 10000000 11110000 | F0 80 F0 80 F0 |
| F | 11110000 10000000 11110000 10000000 10000000 | F0 80 F0 80 80 |

### 2.5 - Timers & Sound

Chip-8 provides 2 timers, a delay timer and a sound timer.

- **Delay timer (DT):** Active whenever DT is non-zero. Subtracts 1 from DT at 60Hz. When DT reaches 0, it deactivates.
- **Sound timer (ST):** Active whenever ST is non-zero. Decrements at 60Hz. As long as ST > 0, the Chip-8 buzzer sounds. When ST reaches zero, the sound timer deactivates.

The sound produced has only one tone. The frequency is decided by the interpreter author.

## 3.0 - Chip-8 Instructions

The original implementation includes 36 different instructions. Super Chip-48 added an additional 10, for a total of 46.

All instructions are 2 bytes long and stored most-significant-byte first. In memory, the first byte of each instruction should be located at an even address.

Variables used:
- **nnn or addr** — 12-bit value, the lowest 12 bits of the instruction
- **n or nibble** — 4-bit value, the lowest 4 bits of the instruction
- **x** — 4-bit value, the lower 4 bits of the high byte of the instruction
- **y** — 4-bit value, the upper 4 bits of the low byte of the instruction
- **kk or byte** — 8-bit value, the lowest 8 bits of the instruction

### 3.1 - Standard Chip-8 Instructions

**0nnn - SYS addr**
Jump to a machine code routine at nnn. Only used on old computers. Ignored by modern interpreters.

**00E0 - CLS**
Clear the display.

**00EE - RET**
Return from a subroutine. The interpreter sets PC to the address at the top of the stack, then subtracts 1 from SP.

**1nnn - JP addr**
Jump to location nnn. PC = nnn.

**2nnn - CALL addr**
Call subroutine at nnn. Increment SP, put current PC on top of stack. PC = nnn.

**3xkk - SE Vx, byte**
Skip next instruction if Vx = kk. If equal, PC += 2.

**4xkk - SNE Vx, byte**
Skip next instruction if Vx != kk. If not equal, PC += 2.

**5xy0 - SE Vx, Vy**
Skip next instruction if Vx = Vy. If equal, PC += 2.

**6xkk - LD Vx, byte**
Set Vx = kk.

**7xkk - ADD Vx, byte**
Set Vx = Vx + kk. Carry flag is not changed.

**8xy0 - LD Vx, Vy**
Set Vx = Vy.

**8xy1 - OR Vx, Vy**
Set Vx = Vx OR Vy. Bitwise OR.

**8xy2 - AND Vx, Vy**
Set Vx = Vx AND Vy. Bitwise AND.

**8xy3 - XOR Vx, Vy**
Set Vx = Vx XOR Vy. Bitwise exclusive OR.

**8xy4 - ADD Vx, Vy**
Set Vx = Vx + Vy, VF = carry. If result > 255, VF = 1, otherwise 0. Only lowest 8 bits stored in Vx.

**8xy5 - SUB Vx, Vy**
Set Vx = Vx - Vy, VF = NOT borrow. If Vx > Vy, VF = 1, otherwise 0. Result stored in Vx.

**8xy6 - SHR Vx {, Vy}**
Set Vx = Vx SHR 1. If LSB of Vx is 1, VF = 1, otherwise 0. Vx = Vx / 2.

**8xy7 - SUBN Vx, Vy**
Set Vx = Vy - Vx, VF = NOT borrow. If Vy > Vx, VF = 1, otherwise 0.

**8xyE - SHL Vx {, Vy}**
Set Vx = Vx SHL 1. If MSB of Vx is 1, VF = 1, otherwise 0. Vx = Vx * 2.

**9xy0 - SNE Vx, Vy**
Skip next instruction if Vx != Vy. If not equal, PC += 2.

**Annn - LD I, addr**
Set I = nnn.

**Bnnn - JP V0, addr**
Jump to location nnn + V0. PC = nnn + V0.

**Cxkk - RND Vx, byte**
Set Vx = random byte AND kk. Random number 0-255 ANDed with kk.

**Dxyn - DRW Vx, Vy, nibble**
Display n-byte sprite starting at memory location I at (Vx, Vy), set VF = collision.

Reads n bytes from memory starting at I. Displayed as sprites at (Vx, Vy). Sprites are XORed onto screen. If any pixels erased, VF = 1, otherwise 0. If sprite is partially outside display coordinates, it wraps around.

**Ex9E - SKP Vx**
Skip next instruction if key with value of Vx is pressed. If key is down, PC += 2.

**ExA1 - SKNP Vx**
Skip next instruction if key with value of Vx is not pressed. If key is up, PC += 2.

**Fx07 - LD Vx, DT**
Set Vx = delay timer value.

**Fx0A - LD Vx, K**
Wait for a key press, store the value in Vx. All execution stops until a key is pressed.

**Fx15 - LD DT, Vx**
Set delay timer = Vx.

**Fx18 - LD ST, Vx**
Set sound timer = Vx.

**Fx1E - ADD I, Vx**
Set I = I + Vx.

**Fx29 - LD F, Vx**
Set I = location of sprite for digit Vx. I points to the hexadecimal sprite for the value of Vx.

**Fx33 - LD B, Vx**
Store BCD representation of Vx in memory at I, I+1, and I+2. Hundreds digit at I, tens at I+1, ones at I+2.

**Fx55 - LD [I], Vx**
Store registers V0 through Vx in memory starting at location I.

**Fx65 - LD Vx, [I]**
Read registers V0 through Vx from memory starting at location I.

### 3.2 - Super Chip-48 Instructions

Listed for reference but not described in this document:

| Opcode | Mnemonic |
|--------|----------|
| 00Cn | SCD nibble |
| 00FB | SCR |
| 00FC | SCL |
| 00FD | EXIT |
| 00FE | LOW |
| 00FF | HIGH |
| Dxy0 | DRW Vx, Vy, 0 |
| Fx30 | LD HF, Vx |
| Fx75 | LD R, Vx |
| Fx85 | LD Vx, R |

## 4.0 - Interpreters

| Title | Version | Author | Platform(s) |
|-------|---------|--------|-------------|
| Chip-48 | 2.20 | Andreas Gustafsson | HP48 |
| Chip8 | 1.1 | Paul Robson | DOS |
| Chip-8 Emulator | 2.0.0 | David Winter | DOS |
| CowChip | 0.1 | Thomas P. Greene | Windows 3.1 |
| DREAM MON | 1.1 | Paul Hayter | Amiga |
| Super Chip-48 | 1.1 | Erik Bryntse (based on Chip-48) | HP48 |
| Vision-8 | 1.0 | Marcel de Kogel | DOS, Adam, MSX, ColecoVision |
