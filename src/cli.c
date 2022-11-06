#include <stdio.h>
#include "cpu.h"

char cli_screen[(65 * 32) + 1] = {' '};

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
    init();
    load("./roms/IBM Logo.ch8");

    for (int i = 0; i < 21; i += 1) {
        //clear_screen();
        debug();
        cycle();
        puts("");
        //puts(screen);
    }

    int j = 0;
    for (int y = 0; y < 32; y += 1) {
        for (int x = 0; x < 64; x += 1) {
            if (screen_buffer[(y * 64) + x]) {
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

