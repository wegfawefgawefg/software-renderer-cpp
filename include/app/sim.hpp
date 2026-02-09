#pragma once

#include "app/app_types.hpp"
#include "app/game.hpp"

namespace app {

void step_game(Game& g, const AppToggles& toggles, const uint8_t* keys, float dt, int mouse_dx,
               int mouse_dy);

} // namespace app
