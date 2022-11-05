/*
 * https://github.com/rxi/microui/pull/48/commits/5307abaa9cb7e29573ab65af341642e4c8941f1d
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#define _STDINT_H_
#include <SDL.h>
//#include <SDL_opengl.h>
#include <assert.h>
#include "renderer.h"
#include "atlas.inl"

#define BUFFER_SIZE 16384
//#define BUFFER_SIZE 3

static SDL_FPoint  tex_buf[BUFFER_SIZE];
static SDL_FPoint  vert_buf[BUFFER_SIZE];
static SDL_Color color_buf[BUFFER_SIZE];
static unsigned int  index_buf[BUFFER_SIZE];

static int width  = 800;
static int height = 600;
static int buf_idx;

static SDL_Window *window;
static SDL_Renderer *renderer;
static SDL_Texture *texture;


void r_init(void) {
  /* init SDL window */
    window = SDL_CreateWindow(
        "title", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED,
        width, height, SDL_WINDOW_OPENGL);

    renderer = SDL_CreateRenderer(
        window,
        -1,
        SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);

    texture = SDL_CreateTexture(
        renderer,
        SDL_PIXELFORMAT_ABGR8888,
        SDL_TEXTUREACCESS_STATIC,
        ATLAS_WIDTH, ATLAS_HEIGHT);

    {
        uint8_t *ptr = SDL_malloc(4 * ATLAS_WIDTH * ATLAS_HEIGHT);
        for (int x = 0; x < ATLAS_WIDTH * ATLAS_HEIGHT; x += 1) {
            ptr[4 * x + 0] = 255;
            ptr[4 * x + 1] = 255;
            ptr[4 * x + 2] = 255;
            ptr[4 * x + 3] = atlas_texture[x];
        }
        SDL_UpdateTexture(texture, NULL, ptr, 4 * ATLAS_WIDTH);
        SDL_free(ptr);
    }

    vert_buf[0].x = 400;
    vert_buf[0].y = 150;
    color_buf[0].r = 255;
    color_buf[0].g = 0;
    color_buf[0].b = 0;
    color_buf[0].a = 255;
    tex_buf[0].x = 0;
    tex_buf[0].y = 0;

    vert_buf[1].x = 200;
    vert_buf[1].y = 450;
    color_buf[1].r = 0;
    color_buf[1].g = 0;
    color_buf[1].b = 255;
    color_buf[1].a = 255;
    tex_buf[1].x = 0;
    tex_buf[1].y = 0;

    vert_buf[2].x = 600;
    vert_buf[2].y = 450;
    color_buf[2].r = 0;
    color_buf[2].g = 255;
    color_buf[2].b = 0;
    color_buf[2].a = 255;
    tex_buf[2].x = 0;
    tex_buf[2].y = 0;

    index_buf[0] = 0;
    index_buf[1] = 1;
    index_buf[2] = 2;

}


static void flush(void) {
  if (buf_idx == 0) { return; }
  buf_idx = 0;
}


static void push_quad(mu_Rect dst, mu_Rect src, mu_Color color) {
    SDL_Rect dest;
    dest.x = dst.x;
    dest.y = dst.y;
    dest.w = dst.w;
    dest.h = dst.h;

    SDL_Rect source;
    source.x = src.x;
    source.y = src.y;
    source.w = src.w;
    source.h = src.h;

    SDL_SetRenderDrawColor(renderer,
            color.a,
            color.b,
            color.g,
            color.r);
    //SDL_RenderCopy(renderer, texture, &source, &dest);
    //SDL_RenderFillRect(renderer, &dest);
    // SDL_RenderDrawRect(renderer, &dest);

        /*
  if (buf_idx == BUFFER_SIZE) { flush(); }

  int texvert_idx = buf_idx *  8;
  int   color_idx = buf_idx * 16;
  int element_idx = buf_idx *  4;
  int   index_idx = buf_idx *  6;
  buf_idx++;

  // update texture buffer
  float x = src.x / (float) ATLAS_WIDTH;
  float y = src.y / (float) ATLAS_HEIGHT;
  float w = src.w / (float) ATLAS_WIDTH;
  float h = src.h / (float) ATLAS_HEIGHT;
  tex_buf[texvert_idx + 0] = x;
  tex_buf[texvert_idx + 1] = y;
  tex_buf[texvert_idx + 2] = x + w;
  tex_buf[texvert_idx + 3] = y;
  tex_buf[texvert_idx + 4] = x;
  tex_buf[texvert_idx + 5] = y + h;
  tex_buf[texvert_idx + 6] = x + w;
  tex_buf[texvert_idx + 7] = y + h;

  // update vertex buffer
  vert_buf[texvert_idx + 0] = dst.x;
  vert_buf[texvert_idx + 1] = dst.y;
  vert_buf[texvert_idx + 2] = dst.x + dst.w;
  vert_buf[texvert_idx + 3] = dst.y;
  vert_buf[texvert_idx + 4] = dst.x;
  vert_buf[texvert_idx + 5] = dst.y + dst.h;
  vert_buf[texvert_idx + 6] = dst.x + dst.w;
  vert_buf[texvert_idx + 7] = dst.y + dst.h;

  // update color buffer
  memcpy(color_buf + color_idx +  0, &color, 4);
  memcpy(color_buf + color_idx +  4, &color, 4);
  memcpy(color_buf + color_idx +  8, &color, 4);
  memcpy(color_buf + color_idx + 12, &color, 4);

  // update index buffer
  index_buf[index_idx + 0] = element_idx + 0;
  index_buf[index_idx + 1] = element_idx + 1;
  index_buf[index_idx + 2] = element_idx + 2;
  index_buf[index_idx + 3] = element_idx + 2;
  index_buf[index_idx + 4] = element_idx + 3;
  index_buf[index_idx + 5] = element_idx + 1;
  */
}


void r_draw_rect(mu_Rect rect, mu_Color color) {
  push_quad(rect, atlas[ATLAS_WHITE], color);
}


void r_draw_text(const char *text, mu_Vec2 pos, mu_Color color) {
  mu_Rect dst = { pos.x, pos.y, 0, 0 };
  for (const char *p = text; *p; p++) {
    if ((*p & 0xc0) == 0x80) { continue; }
    int chr = mu_min((unsigned char) *p, 127);
    mu_Rect src = atlas[ATLAS_FONT + chr];
    dst.w = src.w;
    dst.h = src.h;
    push_quad(dst, src, color);
    dst.x += dst.w;
  }
}


void r_draw_icon(int id, mu_Rect rect, mu_Color color) {
  mu_Rect src = atlas[id];
  int x = rect.x + (rect.w - src.w) / 2;
  int y = rect.y + (rect.h - src.h) / 2;
  push_quad(mu_rect(x, y, src.w, src.h), src, color);
}


int r_get_text_width(const char *text, int len) {
  int res = 0;
  for (const char *p = text; *p && len--; p++) {
    if ((*p & 0xc0) == 0x80) { continue; }
    int chr = mu_min((unsigned char) *p, 127);
    res += atlas[ATLAS_FONT + chr].w;
  }
  return res;
}


int r_get_text_height(void) {
  return 18;
}


void r_set_clip_rect(mu_Rect rect) {
  flush();
  SDL_Rect r;
  r.x = rect.x;
  r.y = rect.y;
  r.w = rect.w;
  r.h = rect.h;
  SDL_RenderSetClipRect(renderer, &r);
}


void r_clear(mu_Color clr) {
  flush();
  SDL_SetRenderDrawColor(renderer, 255, 200, 180, 255);
  SDL_RenderClear(renderer);
}


void r_present(void) {
    float *vert_ptr = (float*)vert_buf;
    float *tex_ptr = (float*)tex_buf;
  SDL_RenderGeometryRaw(renderer, texture,
          vert_ptr,  sizeof(vert_buf[0]),
          color_buf, sizeof(color_buf[0]),
          tex_ptr,   sizeof(tex_buf[0]),
          3,
          index_buf, 3,
          sizeof(index_buf[0]));
  SDL_RenderPresent(renderer);
  flush();
}

