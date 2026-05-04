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

#define false 0
#define true (!0)

struct chip8_cpu {
    u8  memory[0x1000];
    u16 stack[0x10];

    u8  v[0x10];
    u16 i;

    u16 stack_pointer;
    u16 program_counter;

    u8  delay_timer;
    u8  sound_timer;

    u8  keys[16];
    u8  waiting_for_key;
    u8  key_register;

    char screen_buffer[64 * 32];
};

void dis(struct chip8_cpu *cpu, u16 pc, char *string, size_t string_capacity);

#include <time.h>
static int rng_seeded = 0;

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
    memcpy(cpu->memory + 0x50, font_data, sizeof(font_data));
    if (!rng_seeded) {
        srand((unsigned int)time(NULL));
        rng_seeded = 1;
    }
}

void
debug(struct chip8_cpu *cpu) {
    int j = 0;
    char sep = '1';
    for (int j = 0; j < 16; j += 1) {
        sep = ((j - 7) % 8) == 0 ? '\n' : ' ';
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
    u8 x = cpu->v[vx] & 63;
    u8 y = cpu->v[vy] & 31;
    u8 pixel;
    cpu->v[0xf] = 0;

    for (int row = 0; row < n; row++) {
        c = cpu->memory[nnn + row];
        for (int col = 0; col < 8; col++) {
            if ((c & (0x80 >> col)) != 0) {
                int px = (x + col) & 63;
                int py = (y + row) & 31;
                int idx = py * 64 + px;
                if (cpu->screen_buffer[idx]) {
                    cpu->screen_buffer[idx] = 0;
                    cpu->v[0xf] = 1;
                } else {
                    cpu->screen_buffer[idx] = '#';
                }
            }
        }
    }
}

void
cycle(struct chip8_cpu *cpu) {
    u16 op;
    u8 vx, vy, n, nn;
    u16 nnn;
    char dis_buffer[255] = {0};

    if (cpu->waiting_for_key) return;

    op = peek16(cpu, cpu->program_counter);

    dis(cpu, cpu->program_counter, dis_buffer, sizeof(dis_buffer));
    //printf("op: 0x%04x\n%s\n", op, dis_buffer);

    cpu->program_counter += 2;

    switch (op & 0xf000) {
    case 0x0000:
        switch (op & 0x0fff) {
        case 0x00e0:
            memset(cpu->screen_buffer, 0, sizeof(cpu->screen_buffer));
            break;

        case 0x00ee:
            cpu->stack_pointer--;
            cpu->program_counter = cpu->stack[cpu->stack_pointer];
            break;

        default:
            die("todo");
            break;
        }
        break;

    case 0x1000:
        nnn = op & 0xfff;
        cpu->program_counter = nnn;
        break;

    case 0x2000:
        nnn = op & 0xfff;
        cpu->stack[cpu->stack_pointer] = cpu->program_counter;
        cpu->stack_pointer++;
        cpu->program_counter = nnn;
        break;

    case 0x3000:
        vx = (op & 0x0f00) >> 8;
        nn = op & 0x00ff;
        if (cpu->v[vx] == nn) cpu->program_counter += 2;
        break;

    case 0x4000:
        vx = (op & 0x0f00) >> 8;
        nn = op & 0x00ff;
        if (cpu->v[vx] != nn) cpu->program_counter += 2;
        break;

    case 0x5000:
        vx = (op & 0x0f00) >> 8;
        vy = (op & 0x00f0) >> 4;
        if (cpu->v[vx] == cpu->v[vy]) cpu->program_counter += 2;
        break;

    case 0x6000:
        vx = (op & 0xf00) >> 8;
        nn = op & 0xff;
        cpu->v[vx] = nn;
        break;

    case 0x7000:
        vx = (op & 0xf00) >> 8;
        nn = op & 0xff;
        cpu->v[vx] += nn;
        break;

    case 0x8000:
        vx = (op & 0x0f00) >> 8;
        vy = (op & 0x00f0) >> 4;
        switch (op & 0x000f) {
        case 0x0: cpu->v[vx] = cpu->v[vy]; break;
        case 0x1: cpu->v[vx] |= cpu->v[vy]; break;
        case 0x2: cpu->v[vx] &= cpu->v[vy]; break;
        case 0x3: cpu->v[vx] ^= cpu->v[vy]; break;
        case 0x4: {
            u16 sum = cpu->v[vx] + cpu->v[vy];
            cpu->v[0xf] = sum > 0xff ? 1 : 0;
            cpu->v[vx] = sum & 0xff;
            break;
        }
        case 0x5:
            cpu->v[0xf] = cpu->v[vx] >= cpu->v[vy] ? 1 : 0;
            cpu->v[vx] -= cpu->v[vy];
            break;
        case 0x6:
            cpu->v[0xf] = cpu->v[vx] & 0x1;
            cpu->v[vx] >>= 1;
            break;
        case 0x7:
            cpu->v[0xf] = cpu->v[vy] >= cpu->v[vx] ? 1 : 0;
            cpu->v[vx] = cpu->v[vy] - cpu->v[vx];
            break;
        case 0xe:
            cpu->v[0xf] = (cpu->v[vx] & 0x80) >> 7;
            cpu->v[vx] <<= 1;
            break;
        default:
            die("unknown 8xy op");
            break;
        }
        break;

    case 0x9000:
        vx = (op & 0x0f00) >> 8;
        vy = (op & 0x00f0) >> 4;
        if (cpu->v[vx] != cpu->v[vy]) cpu->program_counter += 2;
        break;

    case 0xa000:
        nnn = op & 0xfff;
        cpu->i = nnn;
        break;

    case 0xb000:
        nnn = op & 0xfff;
        cpu->program_counter = nnn + cpu->v[0];
        break;

    case 0xc000:
        vx = (op & 0x0f00) >> 8;
        nn = op & 0x00ff;
        cpu->v[vx] = (rand() % 256) & nn;
        break;

    case 0xd000:
        vx = (op & 0xf00) >> 8;
        vy = (op & 0xf0)  >> 4;
        n = op & 0xf;
        draw(cpu, vx, vy, n);
        break;

    case 0xe000:
        vx = (op & 0x0f00) >> 8;
        switch (op & 0x00ff) {
        case 0x9e:
            if (cpu->keys[cpu->v[vx] & 0x0f]) cpu->program_counter += 2;
            break;
        case 0xa1:
            if (!cpu->keys[cpu->v[vx] & 0x0f]) cpu->program_counter += 2;
            break;
        default:
            die("unknown e op");
            break;
        }
        break;

    case 0xf000:
        vx = (op & 0x0f00) >> 8;
        switch (op & 0x00ff) {
        case 0x07:
            cpu->v[vx] = cpu->delay_timer;
            break;
        case 0x0a:
            cpu->waiting_for_key = 1;
            cpu->key_register = vx;
            break;
        case 0x15:
            cpu->delay_timer = cpu->v[vx];
            break;
        case 0x18:
            cpu->sound_timer = cpu->v[vx];
            break;
        case 0x1e:
            cpu->i += cpu->v[vx];
            break;
        case 0x29:
            cpu->i = 0x50 + (cpu->v[vx] & 0x0f) * 5;
            break;
        case 0x33:
            cpu->memory[cpu->i]     = cpu->v[vx] / 100;
            cpu->memory[cpu->i + 1] = (cpu->v[vx] / 10) % 10;
            cpu->memory[cpu->i + 2] = cpu->v[vx] % 10;
            break;
        case 0x55:
            for (int j = 0; j <= vx; j++) {
                cpu->memory[cpu->i + j] = cpu->v[j];
            }
            break;
        case 0x65:
            for (int j = 0; j <= vx; j++) {
                cpu->v[j] = cpu->memory[cpu->i + j];
            }
            break;
        default:
            die("unknown f op");
            break;
        }
        break;

    default:
        die("unknown op");
        break;
    }
}

void
dis(struct chip8_cpu *cpu, u16 pc, char *string, size_t string_capacity) {
    u16 op;
    u8 vx, vy, n, nn;
    u16 nnn;

    op = peek16(cpu, pc);

    switch (op & 0xf000) {
    case 0x0000:
        switch (op & 0x0fff) {
        case 0x00e0:
            snprintf(string, string_capacity, "clear screen");
            break;

        case 0x00ee:
            snprintf(string, string_capacity, "return");
            break;

        default:
            snprintf(string, string_capacity, "TODO multiple ops %04x", op);
            break;
        }
        break;

    case 0x1000:
        nnn = op & 0xfff;

        if (cpu->program_counter == nnn)
            snprintf(string, string_capacity, "jp $%0x (lockup)", nnn);
        else
            snprintf(string, string_capacity, "jp $%0x", nnn);

        break;

    case 0x2000:
        snprintf(string, string_capacity, "TODO call nnn %04x", op);
        break;

    case 0x3000:
        snprintf(string, string_capacity, "TODO se vx, byte %04x", op);
        break;

    case 0x4000:
        snprintf(string, string_capacity, "TODO sne vx, byte %04x", op);
        break;

    case 0x5000:
        snprintf(string, string_capacity, "TODO se vx, vy %04x", op);
        break;

    case 0x6000:
        vx = (op & 0xf00) >> 8;
        nn = op & 0xff;
        snprintf(string, string_capacity, "ld v%x, $%02x", vx, nn);
        break;

    case 0x7000:
        vx = (op & 0xf00) >> 8;
        nn = op & 0xff;
        snprintf(string, string_capacity, "add v%x, $%02x", vx, nn);
        break;

    case 0x8000:
        snprintf(string, string_capacity, "TODO multiple ops %04x", op);
        break;

    case 0x9000:
        snprintf(string, string_capacity, "TODO sne vx, vy %04x", op);
        break;

    case 0xa000:
        nnn = op & 0xfff;
        snprintf(string, string_capacity, "ld i, $%03x", nnn);
        break;

    case 0xb000:
        snprintf(string, string_capacity, "TODO jp v0, addr %04x", op);
        break;

    case 0xc000:
        snprintf(string, string_capacity, "TODO rnd vx, byte %04x", op);
        break;

    case 0xd000:
        vx = (op & 0xf00) >> 8;
        vy = (op & 0xf0)  >> 4;
        n = op & 0xf;
        u8 x = cpu->v[vx];
        u8 y = cpu->v[vy];
        snprintf(string, string_capacity, "draw x=v%x($%02x), y=v%x($%02x), n=$%x, i=$%03x", vx, x, vy, y, n, cpu->i);
        break;

    case 0xe000:
        vx = (op & 0x0f00) >> 8;
        switch (op & 0x00ff) {
        case 0x9e:
            snprintf(string, string_capacity, "skp v%x", vx);
            break;
        case 0xa1:
            snprintf(string, string_capacity, "sknp v%x", vx);
            break;
        default:
            snprintf(string, string_capacity, "TODO multiple ops %04x", op);
            break;
        }
        break;

    case 0xf000:
        vx = (op & 0x0f00) >> 8;
        switch (op & 0x00ff) {
        case 0x07: snprintf(string, string_capacity, "ld v%x, dt", vx); break;
        case 0x0a: snprintf(string, string_capacity, "ld v%x, k", vx); break;
        case 0x15: snprintf(string, string_capacity, "ld dt, v%x", vx); break;
        case 0x18: snprintf(string, string_capacity, "ld st, v%x", vx); break;
        case 0x1e: snprintf(string, string_capacity, "add i, v%x", vx); break;
        case 0x29: snprintf(string, string_capacity, "ld f, v%x", vx); break;
        case 0x33: snprintf(string, string_capacity, "ld b, v%x", vx); break;
        case 0x55: snprintf(string, string_capacity, "ld [i], v%x", vx); break;
        case 0x65: snprintf(string, string_capacity, "ld v%x, [i]", vx); break;
        default: snprintf(string, string_capacity, "TODO multiple ops %04x", op); break;
        }
        break;

    default:
        snprintf(string, string_capacity, "TODO multiple ops %04x", op);
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

    fread(cpu->memory + 0x200, 1, 0xE00, fp);

    printf("%s\n", fullname);
    /*die("load2");*/
}

