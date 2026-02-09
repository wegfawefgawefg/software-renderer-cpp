#include "app/hud.hpp"

#include "sr/gfx/font5x7.hpp"

#include <cstdio>

namespace app {

void hud_draw(sr::gfx::Framebuffer& fb, const AppToggles& toggles, const FpsCounter& fps) {
    if (!toggles.show_fps)
        return;
    char buf[64];
    std::snprintf(buf, sizeof(buf), "FPS: %.1f", double(fps.value));
    sr::gfx::draw_text_5x7(fb, 8, 8, buf, 0xFFFFFFFFu, 2, 1);
}

} // namespace app
