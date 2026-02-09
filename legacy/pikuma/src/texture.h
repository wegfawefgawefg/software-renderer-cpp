#ifndef TEXTURE_H
#define TEXTURE_H

#include <stdint.h>

#include <SDL2/SDL.h>

#include "vec2.h"

////////////////////////////////////////////////////////////////////////////////
// Texture
////////////////////////////////////////////////////////////////////////////////

typedef struct
{
    int width;
    int height;
    uint32_t *pixels;
} Texture;

Texture *texture_new(int width, int height);
void texture_free(Texture *pb);
void texture_print(Texture *pb);
void copy_to_texture(Texture *pb, SDL_Texture *texture);

void texture_set(Texture *pb, int x, int y, uint32_t color);
void texture_set_alpha(Texture *pb, int x, int y, uint32_t color);
uint32_t texture_get(Texture *pb, int x, int y);
void texture_clear(Texture *pb);
void texture_fade(Texture *pb, uint8_t amount);
void texture_fill(Texture *pb, uint32_t color);

Texture *rotate_pixelbuffer(Texture *src, float degrees);
Texture *scale_pixelbuffer(Texture *src, Vec2 scale);

void blit(Texture *src, Texture *dst, int x, int y);
void blit_with_scale(Texture *src, Texture *dst, IVec2 pos, Vec2 scale);
void blit_with_rotation(Texture *src, Texture *dst, IVec2 pos, float angle, Vec2 center_of_rotation);
void blit_with_scale_and_rotation(Texture *src, Texture *dst, IVec2 pos, Vec2 scale, float angle, Vec2 center_of_rotation);
void blit_dumb(Texture *src, Texture *dst, int x, int y);
void blit_letter(Texture *target_pb, Texture *letters_pb, uint8_t ascii_value, int x, int y, int size, uint32_t color);
void blit_string(Texture *target_pb, Texture *letters_pb, const char *str, int x, int y, int size, uint32_t color);

IVec2 calculate_new_top_left(Texture *src, float degrees, Vec2 center_of_rotation);
void color_rotate(Texture *pb, float hue_shift);
Texture *texture_load_from_png(const char *path);
void draw_outline(Texture *pb, uint32_t color);
IVec2 get_center_of_pixelbuffer(Texture *pb);

#endif