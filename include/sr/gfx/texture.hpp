#pragma once

#include <cstdint>
#include <vector>

namespace sr::gfx {

// Immutable ARGB8888 texture in CPU memory (matches framebuffer format).
class Texture {
  public:
    Texture() = default;
    Texture(int w, int h, std::vector<uint32_t> argb, bool has_alpha, uint8_t alpha_min,
            uint8_t alpha_max, float alpha_non_opaque_frac)
        : width_(w), height_(h), pixels_(std::move(argb)), has_alpha_(has_alpha),
          alpha_min_(alpha_min), alpha_max_(alpha_max),
          alpha_non_opaque_frac_(alpha_non_opaque_frac) {}

    int width() const { return width_; }
    int height() const { return height_; }
    const uint32_t* pixels() const { return pixels_.data(); }
    bool has_alpha() const { return has_alpha_; }
    uint8_t alpha_min() const { return alpha_min_; }
    uint8_t alpha_max() const { return alpha_max_; }
    float alpha_non_opaque_frac() const { return alpha_non_opaque_frac_; }

    bool likely_cutout(float min_non_opaque_frac = 0.0025f) const {
        if (!has_alpha_)
            return false;
        if (alpha_non_opaque_frac_ < min_non_opaque_frac)
            return false;
        return (alpha_min_ == 0) && (alpha_max_ == 255);
    }

    uint32_t sample_repeat(float u, float v) const;

  private:
    int width_ = 0;
    int height_ = 0;
    std::vector<uint32_t> pixels_; // ARGB8888 (SDL_PIXELFORMAT_ARGB8888)
    bool has_alpha_ = false;
    uint8_t alpha_min_ = 255;
    uint8_t alpha_max_ = 255;
    float alpha_non_opaque_frac_ = 0.0f;
};

} // namespace sr::gfx
