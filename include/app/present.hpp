#pragma once

#include "sr/gfx/framebuffer.hpp"

#include <SDL2/SDL.h>

namespace app {

void upload_framebuffer(SDL_Texture* screen, const sr::gfx::Framebuffer& fb);
void present_texture(SDL_Renderer* renderer, SDL_Texture* screen, int window_w, int window_h,
                     int src_w, int src_h);

} // namespace app
