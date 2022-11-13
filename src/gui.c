#include <stdio.h>
#include "cpu.h"
#include <stdint.h>
#define _STDINT_H_

#include <SDL.h>
#include <SDL_ttf.h>

#define LINE_STEP 15

SDL_Window *window     = NULL;
SDL_Renderer *renderer = NULL;
SDL_Texture *screen    = NULL;
SDL_Rect r;

TTF_Font *font = NULL;
SDL_Color forecol = { 0xef, 0xef, 0xef, 0xff };


void render_screen(void);
void render_regs(void);
void render_dis(void);
void render_memory(void);
void render_text(TTF_Font *font, char *buffer, SDL_Color *forecol, int x, int y);

struct chip8_cpu *cpu = NULL;

int
main()
{
    SDL_bool gui_running = SDL_TRUE;
    SDL_bool cpu_running = SDL_FALSE;
    int cycles_per_frame = 10000;

    printf("BasePath: %s\n", SDL_GetBasePath());
    printf("PrefPath: %s\n", SDL_GetPrefPath("org", "app"));
    printf("Platform: %s\n", SDL_GetPlatform());

    SDL_Init(SDL_INIT_VIDEO);

    TTF_Init();

    window = SDL_CreateWindow(
            "CHIP 8",
            SDL_WINDOWPOS_CENTERED,
            SDL_WINDOWPOS_CENTERED,
            800, 600,
            SDL_WINDOW_OPENGL);
    if (window == NULL) {
        fprintf(stderr, "create window failed: %s\n", SDL_GetError());
        return 1;
    }

    renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
    if (!renderer) {
        fprintf(stderr, "create renderer failed: %s\n", SDL_GetError());
        return 1;
    }

    screen = SDL_CreateTexture(
            renderer,
            SDL_PIXELFORMAT_RGBA8888,
            SDL_TEXTUREACCESS_STREAMING,
            64, 32);

    if (screen == NULL) {
        fprintf(stderr, "create texture failed: %s\n", SDL_GetError());
        return 1;
    }


    font = TTF_OpenFont("c:\\windows\\fonts\\consola.ttf", 14);
    if (!font) {
        fprintf(stderr, "%s\n", TTF_GetError());
        return 1;
    }

    cpu = malloc(sizeof(struct chip8_cpu));
    init(cpu);
    load(cpu, "./roms/IBM Logo.ch8");

    /*
    for (int i = 0; i < 21; i += 1) {
        debug();
        cycle();
        puts("");
    }
    */


    while (gui_running) {
        SDL_Event event;

        if (cpu_running) {
            for (int i = 0; i < cycles_per_frame; i += 1)
                cycle(cpu);
        }

        while (SDL_PollEvent(&event)) {
            switch (event.type) {
                case SDL_QUIT:
                    gui_running = SDL_FALSE;
                    break;

                case SDL_KEYDOWN:
                    switch (event.key.keysym.sym) {
                        case SDLK_ESCAPE:
                            cpu_running = SDL_FALSE;
                            break;
                        case SDLK_q:
                            gui_running = SDL_FALSE;
                            break;
                        case SDLK_c:
                            cpu_running = SDL_TRUE;
                            break;
                        case SDLK_j:
                            cycle(cpu);
                            break;
                        default:
                            gui_running = SDL_FALSE;
                            break;
                    }
                    break;

                default:
                    break;
            }
        }

        SDL_SetRenderDrawColor(renderer, 20, 20, 20, 255);
        SDL_RenderClear(renderer);


        render_screen();
        render_regs();
        render_memory();
        render_dis();

        SDL_RenderPresent(renderer);

        SDL_Delay(16);
    }

    TTF_CloseFont(font);
    TTF_Quit();

    SDL_DestroyTexture(screen);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();

    return 0;
}

void
render_screen(void) {
    void *pixels;
    int pitch;
    unsigned char *pixel;
    char *in;

    SDL_LockTexture(screen, 0, &pixels, &pitch);
    pixel = pixels;
    in = (char*)&(cpu->screen_buffer[0]);

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

    /* draw border */
    dest.x -= 1;
    dest.y -= 1;
    dest.w += 2;
    dest.h += 2;
    SDL_SetRenderDrawColor(renderer, 40, 40, 40, 255);
    SDL_RenderDrawRect(renderer, &dest);
}

void
render_regs(void) {
#define REG_BUFFER_SIZE 255
    static char buffer[REG_BUFFER_SIZE] = "";

    snprintf(buffer, REG_BUFFER_SIZE,
            "v0=$%02x v1=$%02x v2=$%02x v3=$%02x v4=$%02x v5=$%02x v6=$%02x v7=$%02x",
            cpu->v[0] , cpu->v[1] , cpu->v[2] , cpu->v[3] ,
            cpu->v[4] , cpu->v[5] , cpu->v[6] , cpu->v[7]);
    render_text(font, buffer, &forecol, 345, 15);

    snprintf(buffer, REG_BUFFER_SIZE,
            "v8=$%02x v9=$%02x va=$%02x vb=$%02x vc=$%02x vd=$%02x ve=$%02x vf=$%02x",
            cpu->v[8] , cpu->v[9] , cpu->v[0xa] , cpu->v[0xb],
            cpu->v[0xc] , cpu->v[0xd] , cpu->v[0xe] , cpu->v[0xf]);
    render_text(font, buffer, &forecol, 345, 40);

    snprintf(buffer, REG_BUFFER_SIZE,
        "sp=$%03x pc=$%03x i=$%03x",
        cpu->stack_pointer, cpu->program_counter, cpu->i);
    render_text(font, buffer, &forecol, 345, 65);
}

void
render_dis(void) {
#define DIS_BUFFER_SIZE 255
    static char buffer[DIS_BUFFER_SIZE] = "";
    static char dis_buffer[DIS_BUFFER_SIZE] = "";
    int x = 345;
    int y = 200;
    u16 op = 0;
    u16 pc = cpu->program_counter;

    snprintf(buffer, DIS_BUFFER_SIZE, "addr hex  op");
    render_text(font, buffer, &forecol, x, y);
    y += LINE_STEP;

    snprintf(buffer, DIS_BUFFER_SIZE, "==== ==== ====");
    render_text(font, buffer, &forecol, x, y);
    y += LINE_STEP;

    for (int i = 0; i < 20; i += 1, y += LINE_STEP, pc += 2) {
        dis(cpu, pc, dis_buffer, DIS_BUFFER_SIZE);
        op = peek16(cpu, pc);
        snprintf(buffer, DIS_BUFFER_SIZE, "%04x %04x %s", pc, op, dis_buffer);
        render_text(font, buffer, &forecol, x, y);
    }
}


void
render_memory(void) {
#define MEMORY_BUFFER_SIZE 255
    static char buffer[MEMORY_BUFFER_SIZE] = "";
    int x = 10;
    int y = 200;
    u16 pc = cpu->i;
    u8 b0 = 0;
    u8 b1 = 0;
    u8 b2 = 0;
    u8 b3 = 0;
    u8 b4 = 0;
    u8 b5 = 0;
    u8 b6 = 0;
    u8 b7 = 0;
    static char s[9] = "........";

    snprintf(buffer, MEMORY_BUFFER_SIZE, "addr 0123 4567 89ab cdef ........");
    render_text(font, buffer, &forecol, x, y);
    y += LINE_STEP;

    snprintf(buffer, MEMORY_BUFFER_SIZE, "==== ==== ==== ==== ==== ========");
    render_text(font, buffer, &forecol, x, y);
    y += LINE_STEP;

    for (int i = 0; i < 20; i += 1, y += LINE_STEP) {
#define X(n) b##n = cpu->memory[pc + n]; s[n] = b##n >= ' ' ? b##n : '.';
        X(0)
        X(1)
        X(2)
        X(3)
        X(4)
        X(5)
        X(6)
        X(7)
#undef X
        snprintf(buffer, MEMORY_BUFFER_SIZE, "%04x %02x%02x %02x%02x %02x%02x %02x%02x %s",
                pc,
                b0, b1, b2, b3, b4, b5, b6, b7,
                s);
        pc += 8;
        render_text(font, buffer, &forecol, x, y);
    }
}

void
render_text(TTF_Font *font, char *buffer, SDL_Color *forecol, int x, int y) {
    static SDL_Surface *text = NULL;
    static SDL_Rect regs_rect;
    static SDL_Texture *regs_texture = NULL;

    text = TTF_RenderText_Blended(font, buffer, *forecol);
    regs_rect.x = x;
    regs_rect.y = y;
    regs_rect.w = text->w;
    regs_rect.h = text->h;
    regs_texture = SDL_CreateTextureFromSurface(renderer, text);
    SDL_FreeSurface(text);
    SDL_RenderCopy(renderer, regs_texture, NULL, &regs_rect);
}

