#pragma once

#include "app/app_types.hpp"

#include <SDL2/SDL.h>

namespace app {

struct InputFrame {
    bool quit = false;
    int mouse_dx = 0;
    int mouse_dy = 0;
};

void input_init(const AppToggles& toggles);
InputFrame poll_input(SDL_Renderer* renderer, AppToggles& toggles);

} // namespace app
