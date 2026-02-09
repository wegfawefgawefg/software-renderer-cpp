#pragma once

#include "sr/gfx/texture.hpp"

#include <SDL2/SDL.h>

#include <string>

namespace sr::assets {

sr::gfx::Texture load_texture_rgba8888(SDL_Renderer* renderer, const std::string& path);

} // namespace sr::assets
