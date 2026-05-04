#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "cpu.h"

static struct chip8_cpu cpu;
static int initialized = 0;

static void json_regs(void) {
    printf("{\"v\":[");
    for (int i = 0; i < 16; i++) {
        printf("%d%s", cpu.v[i], i < 15 ? "," : "");
    }
    printf("],\"i\":%d,\"sp\":%d,\"pc\":%d,\"dt\":%d,\"st\":%d}\n",
        cpu.i, cpu.stack_pointer, cpu.program_counter,
        cpu.delay_timer, cpu.sound_timer);
    fflush(stdout);
}

static void json_mem(int addr, int len) {
    printf("{\"bytes\":\"");
    for (int i = 0; i < len && (addr + i) < 0x1000; i++) {
        printf("%02x%s", cpu.memory[addr + i], i < len - 1 ? " " : "");
    }
    printf("\"}\n");
    fflush(stdout);
}

static void json_screen(void) {
    printf("{\"width\":64,\"height\":32,\"pixels\":\"");
    for (int y = 0; y < 32; y++) {
        for (int x = 0; x < 64; x++) {
            putchar(cpu.screen_buffer[y * 64 + x] ? '#' : '.');
        }
    }
    printf("\"}\n");
    fflush(stdout);
}

static void json_dis(int addr, int count) {
    char buf[256];
    printf("{\"lines\":[");
    for (int i = 0; i < count && addr < 0x1000; i++, addr += 2) {
        dis(&cpu, addr, buf, sizeof(buf));
        u16 op = peek16(&cpu, addr);
        printf("%s{\"addr\":%d,\"op\":\"%04x\",\"text\":\"", i > 0 ? "," : "", addr, op);
        /* escape quotes in buf */
        for (char *p = buf; *p; p++) {
            if (*p == '"' || *p == '\\') putchar('\\');
            putchar(*p);
        }
        printf("\"}");
    }
    printf("]}\n");
    fflush(stdout);
}

static void ok(void) {
    printf("{\"ok\":true}\n");
    fflush(stdout);
}

static void err(const char *msg) {
    printf("{\"ok\":false,\"error\":\"");
    for (const char *p = msg; *p; p++) {
        if (*p == '"' || *p == '\\') putchar('\\');
        putchar(*p);
    }
    printf("\"}\n");
    fflush(stdout);
}

int main(int argc, char **argv) {
    (void)argc; (void)argv;
    setvbuf(stdout, NULL, _IONBF, 0);
    setvbuf(stdin, NULL, _IONBF, 0);
    char line[1024];

    while (fgets(line, sizeof(line), stdin)) {
        line[strcspn(line, "\r\n")] = '\0';

        if (strncmp(line, "INIT", 4) == 0) {
            init(&cpu);
            initialized = 1;
            ok();
        }
        else if (strncmp(line, "LOAD ", 5) == 0) {
            if (!initialized) { err("not initialized"); continue; }
            load(&cpu, line + 5);
            ok();
        }
        else if (strcmp(line, "STEP") == 0) {
            if (!initialized) { err("not initialized"); continue; }
            cycle(&cpu);
            printf("{\"ok\":true,\"pc\":%d}\n", cpu.program_counter);
            fflush(stdout);
        }
        else if (strncmp(line, "RUN ", 4) == 0) {
            if (!initialized) { err("not initialized"); continue; }
            int n = atoi(line + 4);
            for (int i = 0; i < n; i++) cycle(&cpu);
            printf("{\"ok\":true,\"pc\":%d}\n", cpu.program_counter);
            fflush(stdout);
        }
        else if (strcmp(line, "REGS") == 0) {
            if (!initialized) { err("not initialized"); continue; }
            json_regs();
        }
        else if (strncmp(line, "MEM ", 4) == 0) {
            if (!initialized) { err("not initialized"); continue; }
            int addr, len;
            char *endptr;
            addr = (int)strtol(line + 4, &endptr, 0);
            len = atoi(endptr);
            if (endptr != line + 4 && len > 0) {
                json_mem(addr, len);
            } else {
                err("usage: MEM <addr> <len>");
            }
        }
        else if (strcmp(line, "SCREEN") == 0) {
            if (!initialized) { err("not initialized"); continue; }
            json_screen();
        }
        else if (strncmp(line, "DIS ", 4) == 0) {
            if (!initialized) { err("not initialized"); continue; }
            int addr, count;
            char *endptr;
            addr = (int)strtol(line + 4, &endptr, 0);
            count = atoi(endptr);
            if (endptr != line + 4 && count > 0) {
                json_dis(addr, count);
            } else {
                err("usage: DIS <addr> <count>");
            }
        }
        else if (strcmp(line, "STATE") == 0) {
            if (!initialized) { err("not initialized"); continue; }
            printf("{\"initialized\":true,\"pc\":%d,\"sp\":%d,\"i\":%d}\n",
                cpu.program_counter, cpu.stack_pointer, cpu.i);
            fflush(stdout);
        }
        else if (strcmp(line, "QUIT") == 0) {
            ok();
            break;
        }
        else {
            err("unknown command");
        }
    }

    return 0;
}
