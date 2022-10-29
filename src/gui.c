#include <stdio.h>
//#include "cpu.h"
#include <stdint.h>
#define _STDINT_H_

#include <SDL.h>

int
main()
{
    puts("hi");
    SDL_ShowSimpleMessageBox(
            SDL_MESSAGEBOX_INFORMATION,
            "title",
            "message",
            NULL);
    /*
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
    */
    return 0;
}

