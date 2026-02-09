#ifndef COLORS_H
#define COLORS_H

#include <stdint.h>
#include "vec3.h"

// Basic colors
#define COLOR_BLACK 0x000000FF
#define COLOR_WHITE 0xFFFFFFFF
#define COLOR_RED 0xFF0000FF
#define COLOR_GREEN 0x00FF00FF
#define COLOR_BLUE 0x0000FFFF
#define COLOR_YELLOW 0xFFFF00FF
#define COLOR_CYAN 0x00FFFFFF
#define COLOR_MAGENTA 0xFF00FFFF

// Shades of gray
#define COLOR_GRAY_DARK 0x404040FF
#define COLOR_GRAY 0x808080FF
#define COLOR_GRAY_LIGHT 0xC0C0C0FF

// Additional colors
#define COLOR_ORANGE 0xFFA500FF
#define COLOR_PINK 0xFFC0CBFF
#define COLOR_PURPLE 0x800080FF
#define COLOR_BROWN 0xA52A2AFF
#define COLOR_NAVY 0x000080FF
#define COLOR_OLIVE 0x808000FF
#define COLOR_TEAL 0x008080FF
#define COLOR_MAROON 0x800000FF

// Pastel colors
#define COLOR_PASTEL_RED 0xFFB3BAFF
#define COLOR_PASTEL_GREEN 0xBAFFB3FF
#define COLOR_PASTEL_BLUE 0xB3BAFFFF
#define COLOR_PASTEL_YELLOW 0xFFFEB3FF
#define COLOR_PASTEL_PURPLE 0xE0B3FFFF
#define COLOR_PASTEL_ORANGE 0xFFD9B3FF

// Neon colors
#define COLOR_NEON_RED 0xFF3333FF
#define COLOR_NEON_GREEN 0x33FF33FF
#define COLOR_NEON_BLUE 0x3333FFFF
#define COLOR_NEON_PINK 0xFF33FFFF
#define COLOR_NEON_YELLOW 0xFFFF33FF
#define COLOR_NEON_PURPLE 0x9D33FFFF

// Semi-transparent colors (50% opacity)
#define COLOR_TRANS_RED 0xFF000080
#define COLOR_TRANS_GREEN 0x00FF0080
#define COLOR_TRANS_BLUE 0x0000FF80
#define COLOR_TRANS_BLACK 0x00000080
#define COLOR_TRANS_WHITE 0xFFFFFF80
#define COLOR_TRANS_PURPLE 0x80008080

uint32_t color_from_rgb(uint8_t r, uint8_t g, uint8_t b);
uint32_t color_from_rgba(uint8_t r, uint8_t g, uint8_t b, uint8_t a);
uint32_t color_set_alpha(uint32_t color, uint8_t alpha);
uint32_t color_add(uint32_t color_a, uint32_t color_b);
uint32_t color_blend(uint32_t color_a, uint32_t color_b);
uint32_t color_fmul(uint32_t color, float scalar);
uint32_t color_fadd(uint32_t color, float scalar);
Vec3 color_to_vec3(uint32_t color);
uint32_t vec3_to_color(Vec3 v);

#endif