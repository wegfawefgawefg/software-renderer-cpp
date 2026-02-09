#include "sr/platform/sdl.hpp"

#include <SDL2/SDL_image.h>
#include <SDL2/SDL_ttf.h>

#include <stdexcept>

namespace sr::platform {

static void sdl_check(bool ok, const char* what) {
    if (ok)
        return;
    throw std::runtime_error(std::string(what) + ": " + SDL_GetError());
}

SdlApp::SdlApp(const WindowConfig& cfg) : width_(cfg.width), height_(cfg.height) {
    sdl_check(SDL_Init(SDL_INIT_VIDEO | SDL_INIT_TIMER) == 0, "SDL_Init");

    if (TTF_Init() != 0) {
        throw std::runtime_error(std::string("TTF_Init: ") + TTF_GetError());
    }

    // PNG/JPG/etc.
    const int img_flags = IMG_INIT_PNG | IMG_INIT_JPG;
    if ((IMG_Init(img_flags) & img_flags) != img_flags) {
        throw std::runtime_error(std::string("IMG_Init: ") + IMG_GetError());
    }

    window_ = SDL_CreateWindow(cfg.title, SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, cfg.width,
                               cfg.height, SDL_WINDOW_SHOWN);
    sdl_check(window_ != nullptr, "SDL_CreateWindow");

    // Prefer accelerated renderer, but fall back to software (useful for headless/dummy drivers).
    {
        const Uint32 render_flags =
            SDL_RENDERER_ACCELERATED | (cfg.vsync ? SDL_RENDERER_PRESENTVSYNC : 0);
        renderer_ = SDL_CreateRenderer(window_, -1, render_flags);
    }
    if (!renderer_) {
        renderer_ = SDL_CreateRenderer(window_, -1, SDL_RENDERER_SOFTWARE);
    }
    sdl_check(renderer_ != nullptr, "SDL_CreateRenderer");
}

SdlApp::~SdlApp() {
    if (renderer_)
        SDL_DestroyRenderer(renderer_);
    if (window_)
        SDL_DestroyWindow(window_);
    IMG_Quit();
    TTF_Quit();
    SDL_Quit();
}

} // namespace sr::platform
