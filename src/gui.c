#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "cpu.h"
#include <stdint.h>
#define _STDINT_H_

#include <windows.h>
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

int win_w = 800;
int win_h = 600;
static int roms_window_open = 0;
static int roms_window_just_opened = 0;
static char rom_filter[64] = "";

SDL_AudioDeviceID audio_device = 0;

static void audio_callback(void *userdata, Uint8 *stream, int len) {
    (void)userdata;
    static int phase = 0;
    for (int i = 0; i < len; i++) {
        stream[i] = (phase < 128) ? 240 : 16;
        phase = (phase + 1) % 256;
    }
}

static int map_key(SDL_Keycode sym) {
    switch (sym) {
        case SDLK_1: return 0x1;
        case SDLK_2: return 0x2;
        case SDLK_3: return 0x3;
        case SDLK_4: return 0xC;
        case SDLK_q: return 0x4;
        case SDLK_w: return 0x5;
        case SDLK_e: return 0x6;
        case SDLK_r: return 0xD;
        case SDLK_a: return 0x7;
        case SDLK_s: return 0x8;
        case SDLK_d: return 0x9;
        case SDLK_f: return 0xE;
        case SDLK_z: return 0xA;
        case SDLK_x: return 0x0;
        case SDLK_c: return 0xB;
        case SDLK_v: return 0xF;
        default: return -1;
    }
}

static int str_contains_ci(const char *haystack, const char *needle) {
    if (!needle[0]) return 1;
    char h[256], n[64];
    int i;
    for (i = 0; haystack[i] && i < 255; i++) h[i] = (haystack[i] >= 'A' && haystack[i] <= 'Z') ? haystack[i] + 32 : haystack[i];
    h[i] = '\0';
    for (i = 0; needle[i] && i < 63; i++) n[i] = (needle[i] >= 'A' && needle[i] <= 'Z') ? needle[i] + 32 : needle[i];
    n[i] = '\0';
    return strstr(h, n) != NULL;
}

void render_screen(void);
void controls_window(mu_Context *ctx);
void regs_window(mu_Context *ctx);
void dis_window(mu_Context *ctx);
void mem_window(mu_Context *ctx);
void roms_window(mu_Context *ctx);

#define MAX_ROMS 256
static char rom_files[MAX_ROMS][256];
static char rom_names[MAX_ROMS][64];
static int rom_count = 0;

static void scan_dir(const char *dir) {
    char pattern[256];
    WIN32_FIND_DATAA fd;
    HANDLE h;

    /* find .ch8 files */
    snprintf(pattern, sizeof(pattern), "%s\\*.ch8", dir);
    h = FindFirstFileA(pattern, &fd);
    if (h != INVALID_HANDLE_VALUE) {
        do {
            if (rom_count >= MAX_ROMS) break;
            snprintf(rom_files[rom_count], 256, "%s\\%s", dir, fd.cFileName);
            strncpy(rom_names[rom_count], fd.cFileName, 63);
            rom_names[rom_count][63] = '\0';
            rom_count++;
        } while (FindNextFileA(h, &fd));
        FindClose(h);
    }

    /* recurse into subdirectories */
    snprintf(pattern, sizeof(pattern), "%s\\*", dir);
    h = FindFirstFileA(pattern, &fd);
    if (h == INVALID_HANDLE_VALUE) return;
    do {
        if (fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) {
            if (strcmp(fd.cFileName, ".") == 0) continue;
            if (strcmp(fd.cFileName, "..") == 0) continue;
            if (rom_count >= MAX_ROMS) break;
            char subdir[256];
            snprintf(subdir, sizeof(subdir), "%s\\%s", dir, fd.cFileName);
            scan_dir(subdir);
        }
    } while (FindNextFileA(h, &fd));
    FindClose(h);
}

static void scan_roms(void) {
    rom_count = 0;
    scan_dir("roms");
}

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
            win_w, win_h,
            SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE);
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
    scan_roms();
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

    {
        SDL_AudioSpec want;
        SDL_zero(want);
        want.freq = 44100;
        want.format = AUDIO_U8;
        want.channels = 1;
        want.samples = 512;
        want.callback = audio_callback;
        audio_device = SDL_OpenAudioDevice(NULL, 0, &want, NULL, 0);
        if (audio_device == 0) {
            fprintf(stderr, "open audio device failed: %s\n", SDL_GetError());
        }
    }

    while (gui_running) {
        SDL_Event e;

        if (cpu_running) {
            for (int i = 0; i < cycles_per_frame; i++)
                cycle(cpu);
        }

        if (cpu->delay_timer > 0) cpu->delay_timer--;
        if (cpu->sound_timer > 0) {
            cpu->sound_timer--;
            if (audio_device) SDL_PauseAudioDevice(audio_device, 0);
        } else {
            if (audio_device) SDL_PauseAudioDevice(audio_device, 1);
        }

        while (SDL_PollEvent(&e)) {
            switch (e.type) {
                case SDL_QUIT:
                    gui_running = SDL_FALSE;
                    break;

                case SDL_WINDOWEVENT:
                    if (e.window.event == SDL_WINDOWEVENT_SIZE_CHANGED) {
                        win_w = e.window.data1;
                        win_h = e.window.data2;
                    }
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
                    int chip8_key = map_key(e.key.keysym.sym);
                    if (chip8_key >= 0) {
                        cpu->keys[chip8_key] = 1;
                        if (cpu->waiting_for_key) {
                            cpu->v[cpu->key_register] = chip8_key;
                            cpu->waiting_for_key = 0;
                        }
                    }
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
                    int chip8_key = map_key(e.key.keysym.sym);
                    if (chip8_key >= 0) {
                        cpu->keys[chip8_key] = 0;
                    }
                    break;
                }
            }
        }

        mu_begin(ctx);
        controls_window(ctx);
        regs_window(ctx);
        dis_window(ctx);
        mem_window(ctx);
        if (roms_window_open) roms_window(ctx);
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
    int w = win_w - 350;
    if (w < 200) w = 200;
    if (mu_begin_window(ctx, "Controls", mu_rect(340, 10, w, 80))) {
        int bw = (mu_get_current_container(ctx)->body.w - 40) / 4;
        if (bw < 50) bw = 50;
        mu_layout_row(ctx, 4, (int[]){bw, bw, bw, bw}, 0);
        if (mu_button(ctx, "Run"))   { cpu_running = SDL_TRUE; }
        if (mu_button(ctx, "Pause")) { cpu_running = SDL_FALSE; }
        if (mu_button(ctx, "Step"))  { cycle(cpu); }
        if (mu_button(ctx, "ROM")) {
            if (!roms_window_open) roms_window_just_opened = 1;
            roms_window_open = !roms_window_open;
        }
        mu_end_window(ctx);
    }
}

void
regs_window(mu_Context *ctx) {
    int w = win_w - 350;
    if (w < 200) w = 200;
    if (mu_begin_window(ctx, "Registers", mu_rect(340, 100, w, 110))) {
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
        snprintf(buf, sizeof(buf), "sp=$%03x pc=$%03x i=$%03x dt=$%02x st=$%02x%s",
            cpu->stack_pointer, cpu->program_counter, cpu->i,
            cpu->delay_timer, cpu->sound_timer,
            cpu->waiting_for_key ? " WAIT" : "");
        mu_label(ctx, buf);
        mu_end_window(ctx);
    }
}

void
dis_window(mu_Context *ctx) {
    int w = win_w - 350;
    int h = win_h - 240;
    if (w < 200) w = 200;
    if (h < 100) h = 100;
    if (mu_begin_window(ctx, "Disassembly", mu_rect(340, 220, w, h))) {
        static char b1[64], b2[64], b3[256];
        u16 pc = cpu->program_counter;
        mu_layout_row(ctx, 3, (int[]){40, 45, -1}, 0);
        mu_label(ctx, "addr"); mu_label(ctx, "hex"); mu_label(ctx, "op");
        mu_label(ctx, "===="); mu_label(ctx, "===="); mu_label(ctx, "========");
        for (int i = 0; i < 20; i++) {
            dis(cpu, pc, b3, sizeof(b3));
            u16 op = peek16(cpu, pc);
            snprintf(b1, sizeof(b1), "%04x", pc);
            snprintf(b2, sizeof(b2), "%04x", op);
            mu_label(ctx, b1);
            mu_label(ctx, b2);
            mu_label(ctx, b3);
            pc += 2;
        }
        mu_end_window(ctx);
    }
}

void
mem_window(mu_Context *ctx) {
    int h = win_h - 220;
    if (h < 100) h = 100;
    if (mu_begin_window(ctx, "Memory", mu_rect(10, 200, 320, h))) {
        static char b1[32], b2[32], b3[32], b4[32], b5[32], b6[32];
        u16 pc = cpu->i;
        mu_layout_row(ctx, 6, (int[]){40, 40, 40, 40, 40, -1}, 0);
        mu_label(ctx, "addr"); mu_label(ctx, "0123"); mu_label(ctx, "4567");
        mu_label(ctx, "89ab"); mu_label(ctx, "cdef"); mu_label(ctx, "........");
        mu_label(ctx, "===="); mu_label(ctx, "===="); mu_label(ctx, "====");
        mu_label(ctx, "===="); mu_label(ctx, "===="); mu_label(ctx, "========");
        for (int i = 0; i < 16; i++) {
            u8 m0 = cpu->memory[pc+0]; u8 m1 = cpu->memory[pc+1];
            u8 m2 = cpu->memory[pc+2]; u8 m3 = cpu->memory[pc+3];
            u8 m4 = cpu->memory[pc+4]; u8 m5 = cpu->memory[pc+5];
            u8 m6 = cpu->memory[pc+6]; u8 m7 = cpu->memory[pc+7];
            snprintf(b1, sizeof(b1), "%04x", pc);
            snprintf(b2, sizeof(b2), "%02x%02x", m0, m1);
            snprintf(b3, sizeof(b3), "%02x%02x", m2, m3);
            snprintf(b4, sizeof(b4), "%02x%02x", m4, m5);
            snprintf(b5, sizeof(b5), "%02x%02x", m6, m7);
            snprintf(b6, sizeof(b6), "%c%c%c%c%c%c%c%c",
                m0 >= ' ' ? m0 : '.', m1 >= ' ' ? m1 : '.',
                m2 >= ' ' ? m2 : '.', m3 >= ' ' ? m3 : '.',
                m4 >= ' ' ? m4 : '.', m5 >= ' ' ? m5 : '.',
                m6 >= ' ' ? m6 : '.', m7 >= ' ' ? m7 : '.');
            mu_label(ctx, b1); mu_label(ctx, b2); mu_label(ctx, b3);
            mu_label(ctx, b4); mu_label(ctx, b5); mu_label(ctx, b6);
            pc += 8;
        }
        mu_end_window(ctx);
    }
}

void
roms_window(mu_Context *ctx) {
    int h = 50 + rom_count * 25;
    if (h > 400) h = 400;
    if (mu_begin_window(ctx, "ROMs", mu_rect(340, 100, 300, h))) {
        mu_layout_row(ctx, 1, (int[]){-1}, 0);
        mu_textbox(ctx, rom_filter, sizeof(rom_filter));
        if (roms_window_just_opened) {
            mu_set_focus(ctx, ctx->last_id);
            roms_window_just_opened = 0;
        }
        for (int i = 0; i < rom_count; i++) {
            if (rom_filter[0] && !str_contains_ci(rom_names[i], rom_filter)) continue;
            if (mu_button(ctx, rom_names[i])) {
                init(cpu);
                load(cpu, rom_files[i]);
                cpu_running = SDL_FALSE;
                roms_window_open = 0;
            }
        }
        mu_end_window(ctx);
    }
}

