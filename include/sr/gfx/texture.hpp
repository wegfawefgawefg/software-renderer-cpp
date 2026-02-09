#pragma once

#include <cstdint>
#include <vector>

namespace sr::gfx {

// Immutable ARGB8888 texture in CPU memory (matches framebuffer format).
class Texture {
 public:
  Texture() = default;
  Texture(int w, int h, std::vector<uint32_t> rgba)
      : width_(w), height_(h), pixels_(std::move(rgba)) {}

  int width() const { return width_; }
  int height() const { return height_; }
  const uint32_t* pixels() const { return pixels_.data(); }

  uint32_t sample_repeat(float u, float v) const;

 private:
  int width_ = 0;
  int height_ = 0;
  std::vector<uint32_t> pixels_;  // ARGB8888 (SDL_PIXELFORMAT_ARGB8888)
};

}  // namespace sr::gfx
