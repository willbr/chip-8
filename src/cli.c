#include <stdio.h>
#include "cpu.h"

char cli_screen[(65 * 32) + 1] = {' '};
struct chip8_cpu *cpu = NULL;

void
clear_screen(void) {
    memset(cli_screen, '.', sizeof(cli_screen));
    int j = 64;
    for (int i = 0; i < 32; i += 1) {
        cli_screen[j] = '\n';
        j += 64;
    }
    cli_screen[sizeof(cli_screen) - 1] = '\0';
}

int
main(int argc, char **argv)
{
    cpu = malloc(sizeof(struct chip8_cpu));
    init(cpu);
    load(cpu, "./roms/IBM Logo.ch8");

    for (int i = 0; i < 21; i += 1) {
        //clear_screen();
        debug(cpu);
        cycle(cpu);
        puts("");
        //puts(screen);
    }

    int j = 0;
    for (int y = 0; y < 32; y += 1) {
        for (int x = 0; x < 64; x += 1) {
            if (cpu->screen_buffer[(y * 64) + x]) {
                cli_screen[j] = '#';
            } else {
                cli_screen[j] = '.';
            }
            j += 1;
        }
        cli_screen[j] = '\n';
        j += 1;
    }
    cli_screen[j] = '\0';

    puts(cli_screen);

    puts("goodbye");
    return 0;
}

