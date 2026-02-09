#include "app/app_types.hpp"
#include "app/cli.hpp"
#include "app/game.hpp"
#include "app/input.hpp"
#include "app/render.hpp"
#include "app/sim.hpp"

#include "sr/gfx/depthbuffer.hpp"
#include "sr/gfx/framebuffer.hpp"
#include "sr/platform/sdl.hpp"
#include "sr/render/renderer.hpp"

#include <SDL2/SDL.h>

#include <algorithm>
#include <cstdint>

int main(int argc, char** argv) {
    // Always render sharp when scaling.
    SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "0"); // nearest

    app::AppConfig cfg;
    app::AppToggles toggles;
    app::FpsCounter fps;

    if (!app::parse_cli(argc, argv, cfg, toggles))
        return 0;

    sr::platform::WindowConfig wc;
    wc.width = cfg.window_w;
    wc.height = cfg.window_h;
    wc.title = "software-renderer-cpp";
    sr::platform::SdlApp app(wc);

    sr::gfx::Framebuffer fb(cfg.render_w, cfg.render_h);
    sr::gfx::DepthBuffer zb(cfg.render_w, cfg.render_h);
    sr::render::Renderer renderer(fb, zb);

    SDL_Texture* screen = SDL_CreateTexture(app.renderer(), SDL_PIXELFORMAT_ARGB8888,
                                            SDL_TEXTUREACCESS_STREAMING, fb.width(), fb.height());
    if (!screen)
        return 1;

    sr::assets::AssetStore store(app.renderer());
    app::Game game = app::init_game(store);

    // Apply initial mouse mode.
    app::input_init(toggles);

    bool running = true;
    uint64_t last = SDL_GetPerformanceCounter();
    while (running) {
        const uint64_t now = SDL_GetPerformanceCounter();
        float dt = float(double(now - last) / double(SDL_GetPerformanceFrequency()));
        last = now;
        dt = std::min(dt, 0.05f);

        const app::InputFrame in = app::poll_input(app.renderer(), toggles);
        if (in.quit)
            running = false;

        // If gravity was just turned off, keep vertical motion frozen.
        if (!toggles.gravity_enabled) {
            game.player.vel.y = 0.0f;
            game.player.grounded = true;
        }

        const uint8_t* keys = SDL_GetKeyboardState(nullptr);
        app::step_game(game, toggles, keys, dt, in.mouse_dx, in.mouse_dy);

        fps.tick(dt);
        app::render_game(renderer, fb, game, toggles, &fps);
        app::present(app.renderer(), screen, fb, app.width(), app.height());
    }

    SDL_DestroyTexture(screen);
    return 0;
}
