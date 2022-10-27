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

u8  key[0x10];
u8  memory[0x1000];
u16 stack[0x10];
u16 sp;
u16 pc;
u16 i;
u8  delay_timer;
u8  sound_timer;
u8  v0, v1, v2, v3,
    v4, v5, v6, v7,
    v8, v9, va, vb,
    vc, vd, ve, vf;

char screen[65 * 32] = "";

void
init(void) {
    sp = 0, pc = 0, i  = 0;
    v0 = 0, v1 = 0, v2 = 0, v3 = 0;
    v4 = 0, v5 = 0, v6 = 0, v7 = 0;
    v8 = 0, v9 = 0, va = 0, va = 0;
    va = 0, va = 0, va = 0, va = 0;

    delay_timer = 0;
    sound_timer = 0;
    memset(&memory, 0, 0x1000);
    clear_screen();
    puts("hello");

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
}

void
cycle(void) {
    u16 op;
    u8 x, y, n, nn;
    u16 nnn;

    op = peek16(pc);

    printf("pc: 0x%04x\n",   pc);
    printf("tick: 0x%04x\n", op);

    switch (op & 0xf000) {
    case 0x0000:
        switch (op & 0x0fff) {
        case 0x00e0:
            clear_screen();
            break;

        case 0x00ee:
            die("return");
            break;

        default:
            nnn = op & 0xfff;
            printf("call 0x%03x\n", nnn);
            die("todo call");
            break;
        }
        break;

    default:
        die("unknown op");
        break;
    }

    pc += 1;
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

    fread(memory, sizeof(memory), 1, fp);

    /*printf("%s\n", fullname);*/
    /*die("load2");*/
}

