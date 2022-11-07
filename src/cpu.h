#include <string.h> /* memset*/
#include <stdlib.h> /* exit */

/*
https://tobiasvl.github.io/blog/write-a-chip-8-emulator/
*/

void die2(const char * const msg, const char * const file_name, const int line_number, const char * const func_name);
#define die(msg) die2(msg, __FILE__, __LINE__, __func__)

typedef unsigned int   u32;
typedef unsigned short u16;
typedef unsigned char  u8;

#define false 0;
#define true (!0);

struct chip8_cpu {
    u8  memory[0x1000];
    u16 stack[0x10];

    u8 v[0x10];
    u16 i;

    u16 stack_pointer;
    u16 program_counter;

    u8  delay_timer;
    u8  sound_timer;

    char screen_buffer[64 * 32];
};

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
init(struct chip8_cpu *cpu) {
    memset(cpu, 0, sizeof(struct chip8_cpu));
    cpu->program_counter = 0x200;
}

void
debug(struct chip8_cpu *cpu) {
    int j = 0;
    char sep = '1';
    for (int j = 0; j < 16; j += 1) {
        sep = (j-7 % 8) == 0 ? '\n' : ' ';
        printf("v%x=$%02x%c",  j, cpu->v[j], sep);
    }
    printf("\nsp=$%03x program_counter=$%03x i=$%03x\n",
            cpu->stack_pointer, cpu->program_counter, cpu->i);
}

u16
peek16(struct chip8_cpu *cpu, u16 address) {
    return (cpu->memory[address] << 8) | cpu->memory[address + 1];
}

void
draw(struct chip8_cpu *cpu, u8 vx, u8 vy, u8 n) {
    u8 c = 0;
    u16 nnn = cpu->i;
    u8 x = cpu->v[vx];
    u8 y = cpu->v[vy];
    u16 offset = 0;

    offset = (y * 64) + x;
    printf("draw x=$%02x, y=$%02x, n=$%x, i=$%03x\n", x, y, n, cpu->i);
    for (int j = 0; j < n; j += 1) {
        c = cpu->memory[nnn];
        if (c & 0x80) cpu->screen_buffer[offset + 0] = '#';
        if (c & 0x40) cpu->screen_buffer[offset + 1] = '#';
        if (c & 0x20) cpu->screen_buffer[offset + 2] = '#';
        if (c & 0x10) cpu->screen_buffer[offset + 3] = '#';
        if (c & 0x08) cpu->screen_buffer[offset + 4] = '#';
        if (c & 0x04) cpu->screen_buffer[offset + 5] = '#';
        if (c & 0x02) cpu->screen_buffer[offset + 6] = '#';
        if (c & 0x01) cpu->screen_buffer[offset + 7] = '#';
        nnn += 1;
        offset += 64;
    }
}

void
cycle(struct chip8_cpu *cpu) {
    u16 op;
    u8 vx, vy, n, nn;
    u16 nnn;

    op = peek16(cpu, cpu->program_counter);
    cpu->program_counter += 2;

    printf("op: 0x%04x\n", op);

    switch (op & 0xf000) {
    case 0x0000:
        switch (op & 0x0fff) {
        case 0x00e0:
            memset(cpu->screen_buffer, sizeof(cpu->screen_buffer), 0);
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
        nnn = op & 0xfff;

        if (cpu->program_counter == nnn)
            printf("lockup\n");

        cpu->program_counter = nnn;
        printf("jp $%0x\n", cpu->program_counter);
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
        cpu->v[vx] = nn;
        printf("ld v%x, $%02x\n", vx, nn);
        break;

    case 0x7000:
        vx = (op & 0xf00) >> 8;
        nn = op & 0xff;
        cpu->v[vx] += nn;
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
        cpu->i = nnn;
        printf("ld i, $%03x\n", cpu->i);
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
        draw(cpu, vx, vy, n);
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
}


void
die2(const char * const msg, const char * const file_name, const int source_line_number, const char * const func_name)
{
    fprintf(stderr, "\nDIE: %s\n\n", msg);
    fprintf(stderr, "%s : %d : %s\n\n", file_name, source_line_number, func_name);
    exit(1);
}


void
load(struct chip8_cpu *cpu, const char *fullname)
{
    FILE *fp = fopen(fullname, "rb");

    if (!fp)
        die("failed to open file");

    fread(cpu->memory + 0x200, 0x500, 1, fp);

    printf("%s\n", fullname);
    /*die("load2");*/
}

