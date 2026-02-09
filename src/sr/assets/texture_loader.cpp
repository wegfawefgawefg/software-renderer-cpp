#include "sr/assets/texture_loader.hpp"

#include <SDL2/SDL_image.h>

#include <algorithm>
#include <stdexcept>
#include <string>
#include <vector>

namespace sr::assets {

sr::gfx::Texture load_texture_rgba8888(SDL_Renderer* renderer, const std::string& path) {
    (void)renderer; // kept for future (GPU textures, etc.)
    SDL_Surface* surf = IMG_Load(path.c_str());
    if (!surf)
        throw std::runtime_error(std::string("IMG_Load failed: ") + IMG_GetError());

    // Store pixels in the same format as the framebuffer (ARGB8888).
    SDL_Surface* conv = SDL_ConvertSurfaceFormat(surf, SDL_PIXELFORMAT_ARGB8888, 0);
    SDL_FreeSurface(surf);
    if (!conv)
        throw std::runtime_error(std::string("SDL_ConvertSurfaceFormat failed: ") + SDL_GetError());

    const int w = conv->w;
    const int h = conv->h;
    std::vector<uint32_t> pixels;
    pixels.resize(size_t(w) * size_t(h));
    bool has_alpha = false;
    uint8_t alpha_min = 255;
    uint8_t alpha_max = 0;
    uint32_t non_opaque = 0;

    // RGBA8888 means each pixel is 4 bytes; pitch may be wider than w*4.
    const uint8_t* src = static_cast<const uint8_t*>(conv->pixels);
    for (int y = 0; y < h; ++y) {
        const uint32_t* row = reinterpret_cast<const uint32_t*>(src + y * conv->pitch);
        for (int x = 0; x < w; ++x) {
            uint32_t p = row[x];
            uint8_t a = uint8_t((p >> 24) & 0xFFu);
            alpha_min = std::min(alpha_min, a);
            alpha_max = std::max(alpha_max, a);
            if (a != 0xFFu) {
                has_alpha = true;
                non_opaque += 1;
            }
            pixels[size_t(y) * size_t(w) + size_t(x)] = p;
        }
    }

    SDL_FreeSurface(conv);
    float non_opaque_frac = 0.0f;
    if (w > 0 && h > 0) {
        non_opaque_frac = float(non_opaque) / float(uint32_t(w) * uint32_t(h));
    }
    return sr::gfx::Texture(w, h, std::move(pixels), has_alpha, alpha_min, alpha_max,
                            non_opaque_frac);
}

} // namespace sr::assets
