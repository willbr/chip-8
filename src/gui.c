#include <stdio.h>
#include "cpu.h"
#include <stdint.h>
#define _STDINT_H_

#include <SDL.h>
#include <SDL_ttf.h>

SDL_Window *window     = NULL;
SDL_Renderer *renderer = NULL;
SDL_Texture *screen    = NULL;
SDL_Rect r;

TTF_Font *font = NULL;
SDL_Color forecol = { 0xff, 0x00, 0x00, 0xff };

SDL_Texture *regs_1_texture = NULL;
SDL_Rect regs_1_rect;
SDL_Texture *regs_2_texture = NULL;
SDL_Rect regs_2_rect;
SDL_Texture *regs_3_texture = NULL;
SDL_Rect regs_3_rect;

#define REG_BUFFER_SIZE 255
char regs_1[REG_BUFFER_SIZE] = "";
char regs_2[REG_BUFFER_SIZE] = "";
char regs_3[REG_BUFFER_SIZE] = "";

void render_regs(void);

int
main()
{
    SDL_bool running = SDL_TRUE;

    puts("hi");
    printf("%s\n", SDL_GetBasePath());
    printf("%s\n", SDL_GetPrefPath("org", "app"));
    printf("%s\n", SDL_GetPlatform());

    SDL_Init(SDL_INIT_VIDEO);

    TTF_Init();

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


    font = TTF_OpenFont("c:\\windows\\fonts\\consola.ttf", 18);
    if (!font) {
        fprintf(stderr, "%s\n", TTF_GetError());
        return 1;
    }

    init();
    load("./roms/IBM Logo.ch8");

    /*
    for (int i = 0; i < 21; i += 1) {
        debug();
        cycle();
        puts("");
    }
    */


    while (running) {
        SDL_Event event;
        while (SDL_PollEvent(&event)) {
            switch (event.type) {
                case SDL_QUIT:
                    running = SDL_FALSE;
                    break;

                case SDL_KEYDOWN:
                    switch (event.key.keysym.sym) {
                        case SDLK_q:
                            running = SDL_FALSE;
                            break;
                        case SDLK_j:
                            cycle();
                            render_regs();
                            break;
                        default:
                            running = SDL_FALSE;
                            break;
                    }
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
        in = (char*)&screen_buffer;

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
        dest.x = 10;
        dest.y = 10;
        dest.w = 64*5;
        dest.h = 32*5;
        SDL_RenderCopy(renderer, screen, NULL, &dest);

        SDL_RenderCopy(renderer, regs_1_texture, NULL, &regs_1_rect);
        SDL_RenderCopy(renderer, regs_2_texture, NULL, &regs_2_rect);
        SDL_RenderCopy(renderer, regs_3_texture, NULL, &regs_3_rect);

        SDL_RenderPresent(renderer);

        SDL_Delay(16);
    }

    TTF_CloseFont(font);
    TTF_Quit();

    SDL_DestroyTexture(screen);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();

    puts("goodbye");

    return 0;
}

void
render_regs(void) {
    SDL_Surface *text = NULL;
    snprintf(regs_1, REG_BUFFER_SIZE,
            "v0=$%02x v1=$%02x v2=$%02x v3=$%02x v4=$%02x v5=$%02x v6=$%02x v7=$%02x"
            , v[0] , v[1] , v[2] , v[3] , v[4] , v[5] , v[6] , v[7]);

    snprintf(regs_2, REG_BUFFER_SIZE,
            "v8=$%02x v9=$%02x va=$%02x vb=$%02x vc=$%02x vd=$%02x ve=$%02x vf=$%02x"
            , v[8] , v[9] , v[0xa] , v[0xb] , v[0xc] , v[0xd] , v[0xe] , v[0xf]);

    snprintf(regs_3, REG_BUFFER_SIZE,
        "sp=$%03x program_counter=$%03x i=$%03x",
        stack_pointer, program_counter, i);

    text = TTF_RenderText_Blended(font, regs_1, forecol);
    regs_1_rect.x = 10;
    regs_1_rect.y = 200;
    regs_1_rect.w = text->w;
    regs_1_rect.h = text->h;
    regs_1_texture = SDL_CreateTextureFromSurface(renderer, text);
    SDL_FreeSurface(text);

    text = TTF_RenderText_Blended(font, regs_2, forecol);
    regs_2_rect.x = 10;
    regs_2_rect.y = 230;
    regs_2_rect.w = text->w;
    regs_2_rect.h = text->h;
    regs_2_texture = SDL_CreateTextureFromSurface(renderer, text);
    SDL_FreeSurface(text);

    text = TTF_RenderText_Blended(font, regs_3, forecol);
    regs_3_rect.x = 10;
    regs_3_rect.y = 260;
    regs_3_rect.w = text->w;
    regs_3_rect.h = text->h;
    regs_3_texture = SDL_CreateTextureFromSurface(renderer, text);
    SDL_FreeSurface(text);
}

