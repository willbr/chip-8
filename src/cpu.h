#include <string.h> /* memset*/
#include <stdlib.h> /* exit */

/*
https://tobiasvl.github.io/blog/write-a-chip-8-emulator/
*/

void die2(const char * const msg, const char * const file_name, const int line_number, const char * const func_name);
#define die(msg) die2(msg, __FILE__, __LINE__, __func__)

void clear_screen(void);

typedef unsigned int   u32;
typedef unsigned short u16;
typedef unsigned char  u8;

#define false 0;
#define true (!0);

u8  key[0x10];
u8  memory[0x1000];
u16 stack[0x10];
u8 v[0x10];
u16 sp;
u16 pc;
u16 i;
u8  delay_timer;
u8  sound_timer;

char screen[(65 * 32) + 1] = "";

u8 font_data[] = {
    0xF0, 0x90, 0x90, 0x90, 0xF0, // 0
    0x20, 0x60, 0x20, 0x20, 0x70, // 1
    0xF0, 0x10, 0xF0, 0x80, 0xF0, // 2
    0xF0, 0x10, 0xF0, 0x10, 0xF0, // 3
    0x90, 0x90, 0xF0, 0x10, 0x10, // 4
    0xF0, 0x80, 0xF0, 0x10, 0xF0, // 5
    0xF0, 0x80, 0xF0, 0x90, 0xF0, // 6
    0xF0, 0x10, 0x20, 0x40, 0x40, // 7
    0xF0, 0x90, 0xF0, 0x90, 0xF0, // 8
    0xF0, 0x90, 0xF0, 0x10, 0xF0, // 9
    0xF0, 0x90, 0xF0, 0x90, 0x90, // A
    0xE0, 0x90, 0xE0, 0x90, 0xE0, // B
    0xF0, 0x80, 0x80, 0x80, 0xF0, // C
    0xE0, 0x90, 0x90, 0x90, 0xE0, // D
    0xF0, 0x80, 0xF0, 0x80, 0xF0, // E
    0xF0, 0x80, 0xF0, 0x80, 0x80  // F
};

void
init(void) {
    sp = 0, pc = 0x200, i  = 0;
    for (int j = 0; i < 16; i += 1)
        v[j] = 0;

    delay_timer = 0;
    sound_timer = 0;

    memset(&memory, 0, 0x1000);
    memcpy(&memory[0x50], font_data, sizeof(font_data));

    clear_screen();
    puts("hello");

}

void
debug(void) {
    int j = 0;
    char sep = '1';
    for (int j = 0; j < 16; j += 1) {
        sep = (j-7 % 8) == 0 ? '\n' : ' ';
        printf("v%x=$%02x%c",  j, v[j], sep);
    }
    printf("\nsp=$%03x pc=$%03x i=$%03x\n", sp, pc, i);
}

u16
peek16(u16 address) {
    return (memory[address] << 8) | memory[address + 1];
}

void
clear_screen(void) {
    memset(screen, '.', sizeof(screen));
    int j = 64;
    for (int i = 0; i < 32; i += 1) {
        screen[j] = '\n';
        j += 65;
    }
    screen[sizeof(screen) - 1] = '\0';
}

void
draw(u8 vx, u8 vy, u8 n) {
    u8 c = 0;
    u16 nnn = i;
    u8 x = v[vx];
    u8 y = v[vy];
    u16 offset = 0;

    offset = (y * 65) + x;
    printf("draw x=$%02x, y=$%02x, n=$%x, i=$%03x\n", x, y, n, i);
    for (int j = 0; j < n; j += 1) {
        c = memory[nnn];
        if (c & 0x80) screen[offset + 0] = '#';
        if (c & 0x40) screen[offset + 1] = '#';
        if (c & 0x20) screen[offset + 2] = '#';
        if (c & 0x10) screen[offset + 3] = '#';
        if (c & 0x08) screen[offset + 4] = '#';
        if (c & 0x04) screen[offset + 5] = '#';
        if (c & 0x02) screen[offset + 6] = '#';
        if (c & 0x01) screen[offset + 7] = '#';
        nnn += 1;
        offset += 65;
    }
}

void
cycle(void) {
    u16 op;
    u8 vx, vy, n, nn;
    u16 nnn;
    u16 pc_offset = 2;
    int jumped = false;

    op = peek16(pc);

    printf("op: 0x%04x\n", op);

    switch (op & 0xf000) {
    case 0x0000:
        switch (op & 0x0fff) {
        case 0x00e0:
            puts("clear screen");
            clear_screen();
            break;

        case 0x00ee:
            die("return");
            break;

        default:
            die("todo");
            break;
        }
        break;

    case 0x1000:
        pc = op & 0xfff;
        jumped = true;
        printf("jp $%0rx\n", pc);
        break;

    case 0x2000:
        die("call nnn");
        break;

    case 0x3000:
        die("se vx, byte");
        break;

    case 0x4000:
        die("sne vx, byte ");
        break;

    case 0x5000:
        die("se vx, vy");
        break;

    case 0x6000:
        vx = (op & 0xf00) >> 8;
        nn = op & 0xff;
        v[vx] = nn;
        printf("ld v%x, $%02x\n", vx, nn);
        break;

    case 0x7000:
        vx = (op & 0xf00) >> 8;
        nn = op & 0xff;
        v[vx] += nn;
        printf("add v%x, $%02x\n", vx, nn);
        break;

    case 0x8000:
        die("multiple ops");
        break;

    case 0x9000:
        die("sne vx, vy");
        break;

    case 0xa000:
        nnn = op & 0xfff;
        i = nnn;
        printf("ld i, $%03x\n", i);
        break;

    case 0xb000:
        die("jp v0, addr");
        break;

    case 0xc000:
        die("rnd vx, byte");
        break;

    case 0xd000:
        vx = (op & 0xf00) >> 8;
        vy = (op & 0xf0)  >> 4;
        n = op & 0xf;
        draw(vx, vy, n);
        break;

    case 0xe000:
        die("multiple ops");
        break;

    case 0xf000:
        die("multiple ops");
        break;

    default:
        die("unknown op");
        break;
    }

    if (!jumped)
        pc += pc_offset;
}


void
die2(const char * const msg, const char * const file_name, const int source_line_number, const char * const func_name)
{
    fprintf(stderr, "\nDIE: %s\n\n", msg);
    fprintf(stderr, "%s : %d : %s\n\n", file_name, source_line_number, func_name);
    exit(1);
}


void
load(const char *fullname)
{
    FILE *fp = fopen(fullname, "rb");

    if (!fp)
        die("failed to open file");

    fread(memory + 0x200, 0x500, 1, fp);

    /*printf("%s\n", fullname);*/
    /*die("load2");*/
}

