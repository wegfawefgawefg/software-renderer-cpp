#include "sr/gfx/framebuffer.hpp"

#include <algorithm>

namespace sr::gfx {

Framebuffer::Framebuffer(int w, int h) : width_(w), height_(h), pixels_(w * h, 0) {}

void Framebuffer::clear(uint32_t argb) {
  std::fill(pixels_.begin(), pixels_.end(), argb);
}

}  // namespace sr::gfx

