#pragma once

namespace app {

struct AppConfig {
    int window_w = 1280;
    int window_h = 720;

    // Crunchy internal render resolution (scaled up to window).
    int render_w = 720;
    int render_h = 480;
};

struct AppToggles {
    bool mouse_look = true;
    bool cull_enabled = true;
    bool flip_winding = false;
    bool castle_double_sided = false;
    bool gravity_enabled = true;
    bool show_fps = true;
};

struct FpsCounter {
    float accum_t = 0.0f;
    int frames = 0;
    float value = 0.0f;

    void tick(float dt) {
        accum_t += dt;
        frames += 1;
        if (accum_t >= 0.25f) {
            value = float(frames) / accum_t;
            accum_t = 0.0f;
            frames = 0;
        }
    }
};

} // namespace app
