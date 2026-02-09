#pragma once

#include <SDL2/SDL.h>

#include <cstdint>
#include <string>

namespace sr::platform {

struct WindowConfig {
    int width = 1280;
    int height = 720;
    const char* title = "software-renderer-cpp";
    bool vsync = true;
};

class SdlApp {
  public:
    explicit SdlApp(const WindowConfig& cfg);
    ~SdlApp();

    SdlApp(const SdlApp&) = delete;
    SdlApp& operator=(const SdlApp&) = delete;

    SDL_Window* window() const { return window_; }
    SDL_Renderer* renderer() const { return renderer_; }

    int width() const { return width_; }
    int height() const { return height_; }

  private:
    SDL_Window* window_ = nullptr;
    SDL_Renderer* renderer_ = nullptr;
    int width_ = 0;
    int height_ = 0;
};

} // namespace sr::platform
