#pragma once

#include "app/app_types.hpp"

#include "sr/gfx/framebuffer.hpp"

namespace app {

void hud_draw(sr::gfx::Framebuffer& fb, const AppToggles& toggles, const FpsCounter& fps);

} // namespace app
