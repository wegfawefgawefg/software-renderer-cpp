#include "app/present.hpp"

#include "app/util.hpp"

#include <cstdint>
#include <cstring>

namespace app {

void upload_framebuffer(SDL_Texture* screen, const sr::gfx::Framebuffer& fb) {
    void* pixels = nullptr;
    int pitch = 0;
    if (SDL_LockTexture(screen, nullptr, &pixels, &pitch) == 0) {
        const int row_bytes = fb.width() * int(sizeof(uint32_t));
        for (int y = 0; y < fb.height(); ++y) {
            std::memcpy(static_cast<uint8_t*>(pixels) + y * pitch, fb.pixels() + y * fb.width(),
                        row_bytes);
        }
        SDL_UnlockTexture(screen);
    }
}

void present_texture(SDL_Renderer* renderer, SDL_Texture* screen, int window_w, int window_h,
                     int src_w, int src_h) {
    SDL_RenderClear(renderer);
    const SDL_Rect dst = app::centered_letterbox_rect(window_w, window_h, src_w, src_h);
    SDL_RenderCopy(renderer, screen, nullptr, &dst);
    SDL_RenderPresent(renderer);
}

} // namespace app
