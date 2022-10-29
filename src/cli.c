#include <stdio.h>
#include "cpu.h"

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
    puts(screen);

    puts("goodbye");
    return 0;
}

