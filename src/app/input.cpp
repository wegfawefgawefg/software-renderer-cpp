#include "app/input.hpp"

#include <SDL2/SDL.h>

namespace app {

static void apply_mouse_mode(const AppToggles& toggles) {
    SDL_SetRelativeMouseMode(toggles.mouse_look ? SDL_TRUE : SDL_FALSE);
    SDL_ShowCursor(toggles.mouse_look ? SDL_DISABLE : SDL_ENABLE);
}

void input_init(const AppToggles& toggles) {
    apply_mouse_mode(toggles);
}

InputFrame poll_input(SDL_Renderer* renderer, AppToggles& toggles) {
    (void)renderer;

    InputFrame in;

    SDL_Event e;
    while (SDL_PollEvent(&e)) {
        if (e.type == SDL_QUIT)
            in.quit = true;

        if (e.type == SDL_KEYDOWN) {
            if (e.key.keysym.sym == SDLK_ESCAPE)
                in.quit = true;

            if (e.key.keysym.sym == SDLK_m) {
                toggles.mouse_look = !toggles.mouse_look;
                apply_mouse_mode(toggles);
            }
            if (e.key.keysym.sym == SDLK_c)
                toggles.cull_enabled = !toggles.cull_enabled;
            if (e.key.keysym.sym == SDLK_v)
                toggles.flip_winding = !toggles.flip_winding;
            if (e.key.keysym.sym == SDLK_t)
                toggles.castle_double_sided = !toggles.castle_double_sided;
            if (e.key.keysym.sym == SDLK_g)
                toggles.gravity_enabled = !toggles.gravity_enabled;
        }

        if (e.type == SDL_MOUSEMOTION && toggles.mouse_look) {
            in.mouse_dx += e.motion.xrel;
            in.mouse_dy += e.motion.yrel;
        }
    }

    return in;
}

} // namespace app
