#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "cpu.h"
#include <stdint.h>
#define _STDINT_H_

#include <SDL.h>
#include "renderer.h"
#include "microui.h"

SDL_Window *window     = NULL;
SDL_Renderer *renderer = NULL;
SDL_Texture *screen_tex = NULL;

struct chip8_cpu *cpu = NULL;
SDL_bool gui_running = SDL_TRUE;
SDL_bool cpu_running = SDL_FALSE;
int cycles_per_frame = 10000;

mu_Context *ctx = NULL;

void render_screen(void);
void controls_window(mu_Context *ctx);
void regs_window(mu_Context *ctx);
void dis_window(mu_Context *ctx);
void mem_window(mu_Context *ctx);

static int text_width(mu_Font font, const char *text, int len) {
    if (len == -1) { len = strlen(text); }
    return r_get_text_width(text, len);
}

static int text_height(mu_Font font) {
    return r_get_text_height();
}

static const char button_map[256] = {
    [ SDL_BUTTON_LEFT   & 0xff ] =  MU_MOUSE_LEFT,
    [ SDL_BUTTON_RIGHT  & 0xff ] =  MU_MOUSE_RIGHT,
    [ SDL_BUTTON_MIDDLE & 0xff ] =  MU_MOUSE_MIDDLE,
};

static const char key_map[256] = {
    [ SDLK_LSHIFT       & 0xff ] = MU_KEY_SHIFT,
    [ SDLK_RSHIFT       & 0xff ] = MU_KEY_SHIFT,
    [ SDLK_LCTRL        & 0xff ] = MU_KEY_CTRL,
    [ SDLK_RCTRL        & 0xff ] = MU_KEY_CTRL,
    [ SDLK_LALT         & 0xff ] = MU_KEY_ALT,
    [ SDLK_RALT         & 0xff ] = MU_KEY_ALT,
    [ SDLK_RETURN       & 0xff ] = MU_KEY_RETURN,
    [ SDLK_BACKSPACE    & 0xff ] = MU_KEY_BACKSPACE,
};

int
main()
{
    printf("BasePath: %s\n", SDL_GetBasePath());
    printf("PrefPath: %s\n", SDL_GetPrefPath("org", "app"));
    printf("Platform: %s\n", SDL_GetPlatform());

    SDL_Init(SDL_INIT_VIDEO);

    window = SDL_CreateWindow(
            "CHIP-8",
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

    r_init_ex(renderer);

    ctx = malloc(sizeof(mu_Context));
    mu_init(ctx);
    ctx->text_width = text_width;
    ctx->text_height = text_height;

    cpu = malloc(sizeof(struct chip8_cpu));
    init(cpu);
    load(cpu, "./roms/IBM Logo.ch8");

    screen_tex = SDL_CreateTexture(
            renderer,
            SDL_PIXELFORMAT_RGBA8888,
            SDL_TEXTUREACCESS_STREAMING,
            64, 32);
    if (screen_tex == NULL) {
        fprintf(stderr, "create screen texture failed: %s\n", SDL_GetError());
        return 1;
    }

    while (gui_running) {
        SDL_Event e;

        if (cpu_running) {
            for (int i = 0; i < cycles_per_frame; i++)
                cycle(cpu);
        }

        while (SDL_PollEvent(&e)) {
            switch (e.type) {
                case SDL_QUIT:
                    gui_running = SDL_FALSE;
                    break;

                case SDL_MOUSEMOTION:
                    mu_input_mousemove(ctx, e.motion.x, e.motion.y);
                    break;

                case SDL_MOUSEWHEEL:
                    mu_input_scroll(ctx, 0, e.wheel.y * -30);
                    break;

                case SDL_TEXTINPUT:
                    mu_input_text(ctx, e.text.text);
                    break;

                case SDL_MOUSEBUTTONDOWN:
                case SDL_MOUSEBUTTONUP: {
                    int b = button_map[e.button.button & 0xff];
                    if (b && e.type == SDL_MOUSEBUTTONDOWN) { mu_input_mousedown(ctx, e.button.x, e.button.y, b); }
                    if (b && e.type == SDL_MOUSEBUTTONUP)   { mu_input_mouseup(ctx, e.button.x, e.button.y, b); }
                    break;
                }

                case SDL_KEYDOWN: {
                    int c = key_map[e.key.keysym.sym & 0xff];
                    if (c) { mu_input_keydown(ctx, c); }
                    switch (e.key.keysym.sym) {
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
                    }
                    break;
                }

                case SDL_KEYUP: {
                    int c = key_map[e.key.keysym.sym & 0xff];
                    if (c) { mu_input_keyup(ctx, c); }
                    break;
                }
            }
        }

        mu_begin(ctx);
        controls_window(ctx);
        regs_window(ctx);
        dis_window(ctx);
        mem_window(ctx);
        mu_end(ctx);

        r_clear(mu_color(20, 20, 20, 255));
        render_screen();

        mu_Command *cmd = NULL;
        while (mu_next_command(ctx, &cmd)) {
            switch (cmd->type) {
                case MU_COMMAND_TEXT:  r_draw_text(cmd->text.str, cmd->text.pos, cmd->text.color); break;
                case MU_COMMAND_RECT:  r_draw_rect(cmd->rect.rect, cmd->rect.color); break;
                case MU_COMMAND_ICON:  r_draw_icon(cmd->icon.id, cmd->icon.rect, cmd->icon.color); break;
                case MU_COMMAND_CLIP:  r_set_clip_rect(cmd->clip.rect); break;
            }
        }

        r_render();
        SDL_RenderPresent(renderer);

        SDL_Delay(16);
    }

    SDL_DestroyTexture(screen_tex);
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

    SDL_LockTexture(screen_tex, 0, &pixels, &pitch);
    pixel = pixels;
    in = (char*)&(cpu->screen_buffer[0]);

    for (int y = 0; y < 32; y++) {
        for (int x = 0; x < 64; x++) {
            if (*in) {
                *pixel = 0x00; pixel++;
                *pixel = 0xff; pixel++;
                *pixel = 0xff; pixel++;
                *pixel = 0xff; pixel++;
            } else {
                *pixel = 0x00; pixel++;
                *pixel = 0x00; pixel++;
                *pixel = 0x00; pixel++;
                *pixel = 0x00; pixel++;
            }
            in++;
        }
    }
    SDL_UnlockTexture(screen_tex);

    SDL_Rect dest;
    dest.x = 10;
    dest.y = 10;
    dest.w = 64*5;
    dest.h = 32*5;
    SDL_RenderCopy(renderer, screen_tex, NULL, &dest);

    /* draw border */
    dest.x -= 1;
    dest.y -= 1;
    dest.w += 2;
    dest.h += 2;
    SDL_SetRenderDrawColor(renderer, 40, 40, 40, 255);
    SDL_RenderDrawRect(renderer, &dest);
}

void
controls_window(mu_Context *ctx) {
    if (mu_begin_window(ctx, "Controls", mu_rect(350, 10, 200, 80))) {
        mu_layout_row(ctx, 3, (int[]){-1, -1, -1}, 0);
        if (mu_button(ctx, "Run"))   { cpu_running = SDL_TRUE; }
        if (mu_button(ctx, "Pause")) { cpu_running = SDL_FALSE; }
        if (mu_button(ctx, "Step"))  { cycle(cpu); }
        mu_end_window(ctx);
    }
}

void
regs_window(mu_Context *ctx) {
    if (mu_begin_window(ctx, "Registers", mu_rect(350, 100, 430, 110))) {
        static char buf[256];
        mu_layout_row(ctx, 1, (int[]){-1}, 0);
        snprintf(buf, sizeof(buf),
            "v0=$%02x v1=$%02x v2=$%02x v3=$%02x v4=$%02x v5=$%02x v6=$%02x v7=$%02x",
            cpu->v[0], cpu->v[1], cpu->v[2], cpu->v[3],
            cpu->v[4], cpu->v[5], cpu->v[6], cpu->v[7]);
        mu_label(ctx, buf);
        snprintf(buf, sizeof(buf),
            "v8=$%02x v9=$%02x va=$%02x vb=$%02x vc=$%02x vd=$%02x ve=$%02x vf=$%02x",
            cpu->v[8], cpu->v[9], cpu->v[10], cpu->v[11],
            cpu->v[12], cpu->v[13], cpu->v[14], cpu->v[15]);
        mu_label(ctx, buf);
        snprintf(buf, sizeof(buf), "sp=$%03x pc=$%03x i=$%03x",
            cpu->stack_pointer, cpu->program_counter, cpu->i);
        mu_label(ctx, buf);
        mu_end_window(ctx);
    }
}

void
dis_window(mu_Context *ctx) {
    if (mu_begin_window(ctx, "Disassembly", mu_rect(350, 220, 430, 350))) {
        static char buf[256];
        static char dis_buf[256];
        u16 pc = cpu->program_counter;
        mu_layout_row(ctx, 1, (int[]){-1}, 0);
        mu_label(ctx, "addr hex  op");
        mu_label(ctx, "==== ==== ====");
        for (int i = 0; i < 20; i++) {
            dis(cpu, pc, dis_buf, sizeof(dis_buf));
            u16 op = peek16(cpu, pc);
            snprintf(buf, sizeof(buf), "%04x %04x %s", pc, op, dis_buf);
            mu_label(ctx, buf);
            pc += 2;
        }
        mu_end_window(ctx);
    }
}

void
mem_window(mu_Context *ctx) {
    if (mu_begin_window(ctx, "Memory", mu_rect(10, 200, 320, 370))) {
        static char buf[256];
        u16 pc = cpu->i;
        mu_layout_row(ctx, 1, (int[]){-1}, 0);
        mu_label(ctx, "addr 0123 4567 89ab cdef ........");
        mu_label(ctx, "==== ==== ==== ==== ==== ========");
        for (int i = 0; i < 16; i++) {
            u8 b0 = cpu->memory[pc+0]; u8 b1 = cpu->memory[pc+1];
            u8 b2 = cpu->memory[pc+2]; u8 b3 = cpu->memory[pc+3];
            u8 b4 = cpu->memory[pc+4]; u8 b5 = cpu->memory[pc+5];
            u8 b6 = cpu->memory[pc+6]; u8 b7 = cpu->memory[pc+7];
            char s[9] = {
                b0 >= ' ' ? b0 : '.', b1 >= ' ' ? b1 : '.',
                b2 >= ' ' ? b2 : '.', b3 >= ' ' ? b3 : '.',
                b4 >= ' ' ? b4 : '.', b5 >= ' ' ? b5 : '.',
                b6 >= ' ' ? b6 : '.', b7 >= ' ' ? b7 : '.', '\0'
            };
            snprintf(buf, sizeof(buf), "%04x %02x%02x %02x%02x %02x%02x %02x%02x %s",
                    pc, b0, b1, b2, b3, b4, b5, b6, b7, s);
            mu_label(ctx, buf);
            pc += 8;
        }
        mu_end_window(ctx);
    }
}
