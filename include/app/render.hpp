#pragma once

#include "app/app_types.hpp"
#include "app/game.hpp"

#include "sr/gfx/framebuffer.hpp"
#include "sr/render/renderer.hpp"

#include <SDL2/SDL.h>

namespace app {

void render_game(sr::render::Renderer& renderer, sr::gfx::Framebuffer& fb, Game& g,
                 const AppToggles& toggles, FpsCounter* fps);

} // namespace app
