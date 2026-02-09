#pragma once

#include "sr/gfx/framebuffer.hpp"

#include <algorithm>
#include <cstdint>

namespace sr::gfx {

// Tiny 5x7 ASCII bitmap font for debug overlays.
// Each glyph is 5 pixels wide, 7 pixels tall. Bits are MSB->LSB across 5 columns.
struct Font5x7 {
    struct Glyph {
        char c;
        uint8_t rows[7];
    };

    static constexpr Glyph kGlyphs[] = {
        {' ', {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}},
        {':', {0x00, 0x04, 0x00, 0x00, 0x04, 0x00, 0x00}},
        {'.', {0x00, 0x00, 0x00, 0x00, 0x00, 0x06, 0x06}},
        {'0', {0x0E, 0x11, 0x13, 0x15, 0x19, 0x11, 0x0E}},
        {'1', {0x04, 0x0C, 0x04, 0x04, 0x04, 0x04, 0x0E}},
        {'2', {0x0E, 0x11, 0x01, 0x02, 0x04, 0x08, 0x1F}},
        {'3', {0x1F, 0x02, 0x04, 0x02, 0x01, 0x11, 0x0E}},
        {'4', {0x02, 0x06, 0x0A, 0x12, 0x1F, 0x02, 0x02}},
        {'5', {0x1F, 0x10, 0x1E, 0x01, 0x01, 0x11, 0x0E}},
        {'6', {0x06, 0x08, 0x10, 0x1E, 0x11, 0x11, 0x0E}},
        {'7', {0x1F, 0x01, 0x02, 0x04, 0x08, 0x08, 0x08}},
        {'8', {0x0E, 0x11, 0x11, 0x0E, 0x11, 0x11, 0x0E}},
        {'9', {0x0E, 0x11, 0x11, 0x0F, 0x01, 0x02, 0x0C}},
        {'F', {0x1F, 0x10, 0x10, 0x1E, 0x10, 0x10, 0x10}},
        {'P', {0x1E, 0x11, 0x11, 0x1E, 0x10, 0x10, 0x10}},
        {'S', {0x0F, 0x10, 0x10, 0x0E, 0x01, 0x01, 0x1E}},
    };

    static constexpr const Glyph* find(char c) {
        for (const auto& g : kGlyphs) {
            if (g.c == c)
                return &g;
        }
        return nullptr;
    }
};

inline void draw_char_5x7(Framebuffer& fb, int x, int y, char c, uint32_t argb, int scale = 2) {
    if (scale < 1)
        scale = 1;
    const auto* g = Font5x7::find(c);
    if (!g)
        return;

    const int w = fb.width();
    const int h = fb.height();
    uint32_t* pix = fb.pixels();

    for (int row = 0; row < 7; ++row) {
        uint8_t bits = g->rows[row];
        for (int col = 0; col < 5; ++col) {
            const bool on = (bits & (1u << (4 - col))) != 0;
            if (!on)
                continue;

            const int px0 = x + col * scale;
            const int py0 = y + row * scale;
            const int px1 = std::min(px0 + scale, w);
            const int py1 = std::min(py0 + scale, h);
            for (int py = std::max(py0, 0); py < py1; ++py) {
                for (int px = std::max(px0, 0); px < px1; ++px) {
                    pix[py * w + px] = argb;
                }
            }
        }
    }
}

inline void draw_text_5x7(Framebuffer& fb, int x, int y, const char* s, uint32_t argb,
                          int scale = 2, int spacing = 1) {
    if (!s)
        return;
    if (scale < 1)
        scale = 1;
    if (spacing < 0)
        spacing = 0;

    int cx = x;
    for (const char* p = s; *p; ++p) {
        draw_char_5x7(fb, cx, y, *p, argb, scale);
        cx += (5 * scale) + spacing * scale;
    }
}

} // namespace sr::gfx
