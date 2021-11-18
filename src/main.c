#include <stdio.h>
#include "cpu.h"

int
main(int argc, char **argv)
{
    init();
    load("../roms/IBM Logo.ch8");
    cycle();
    puts("goodbye");
    return 0;
}

