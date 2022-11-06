#include <stdio.h>
#include "cpu.h"
#include <stdint.h>
#define _STDINT_H_

#include <SDL.h>

SDL_Window *window     = NULL;
SDL_Renderer *renderer = NULL;
SDL_Texture *screen    = NULL;
SDL_Rect r;

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

    screen = SDL_CreateTexture(
            renderer,
            SDL_PIXELFORMAT_RGBA8888,
            SDL_TEXTUREACCESS_STREAMING,
            64, 32);

    if (screen == NULL) {
        printf("create texture failed: %s\n", SDL_GetError());
        return 1;
    }

    init();
    load("./roms/IBM Logo.ch8");

    for (int i = 0; i < 21; i += 1) {
        debug();
        cycle();
        puts("");
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

        void *pixels;
        int pitch;
        unsigned char *pixel;
        char *in;

        SDL_LockTexture(screen, 0, &pixels, &pitch);
        pixel = pixels;
        in = &screen_buffer;

        for (int y = 0; y < 32; y += 1) {
            for (int x = 0; x < 64; x += 1) {
                if (*in) {
                    *pixel = 0x00; pixel += 1;
                    *pixel = 0xff; pixel += 1;
                    *pixel = 0xff; pixel += 1;
                    *pixel = 0xff; pixel += 1;
                } else {
                    *pixel = 0x00; pixel += 1;
                    *pixel = 0x00; pixel += 1;
                    *pixel = 0x00; pixel += 1;
                    *pixel = 0x00; pixel += 1;
                }
                in += 1;
            }
        }
        SDL_UnlockTexture(screen);

        SDL_Rect dest;
        dest.x = 200;
        dest.y = 200;
        dest.w = 64*4;
        dest.h = 32*4;
        SDL_RenderCopy(renderer, screen, NULL, &dest);

        SDL_RenderPresent(renderer);

        SDL_Delay(16);
    }




    SDL_DestroyTexture(screen);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();

    puts("goodbye");

    return 0;
}

