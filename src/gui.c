#include <stdio.h>
//#include "cpu.h"
#include <stdint.h>
#define _STDINT_H_

#include <SDL.h>

SDL_Window *window     = NULL;
SDL_Renderer *renderer = NULL;

int
main()
{
    SDL_bool running = SDL_TRUE;

    puts("hi");
    printf("%s\n", SDL_GetBasePath());
    printf("%s\n", SDL_GetPrefPath("org", "app"));
    printf("%s\n", SDL_GetPlatform());

    SDL_Init(SDL_INIT_VIDEO);

    window = SDL_CreateWindow(
            "title",
            SDL_WINDOWPOS_CENTERED,
            SDL_WINDOWPOS_CENTERED,
            800, 600,
            SDL_WINDOW_OPENGL);
    if (window == NULL) {
        printf("create window failed: %s\n", SDL_GetError());
        return 1;
    }

    renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
    if (!renderer) {
        printf("create renderer failed: %s\n", SDL_GetError());
        return 1;
    }


    while (running) {
        SDL_Event event;
        while (SDL_PollEvent(&event)) {
            switch (event.type) {
                case SDL_QUIT:
                    running = SDL_FALSE;
                    break;

                case SDL_KEYDOWN:
                    running = SDL_FALSE;
                    break;

                default:
                    break;
            }
        }

        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
        SDL_RenderClear(renderer);

        SDL_SetRenderDrawColor(renderer, 0, 255, 0, 255);
        SDL_RenderDrawPoint(renderer, 300, 300);

        SDL_RenderPresent(renderer);

        SDL_Delay(16);
    }



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

    */

    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();

    puts("goodbye");

    return 0;
}

