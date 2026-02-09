#include "sr/gfx/texture.hpp"

#include <cmath>

namespace sr::gfx {

uint32_t Texture::sample_repeat(float u, float v) const {
  if (width_ <= 0 || height_ <= 0 || pixels_.empty()) return 0;

  // Repeat wrap into [0,1).
  u = u - std::floor(u);
  v = v - std::floor(v);

  int x = int(u * float(width_ - 1));
  int y = int(v * float(height_ - 1));
  if (x < 0) x = 0;
  if (y < 0) y = 0;
  if (x >= width_) x = width_ - 1;
  if (y >= height_) y = height_ - 1;
  return pixels_[y * width_ + x];
}

}  // namespace sr::gfx

