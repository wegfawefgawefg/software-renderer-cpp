#pragma once

#include <cstdint>
#include <vector>

namespace sr::gfx {

// CPU pixel buffer: ARGB8888.
class Framebuffer {
  public:
    Framebuffer(int w, int h);

    int width() const { return width_; }
    int height() const { return height_; }

    void clear(uint32_t argb);
    uint32_t* pixels() { return pixels_.data(); }
    const uint32_t* pixels() const { return pixels_.data(); }

  private:
    int width_ = 0;
    int height_ = 0;
    std::vector<uint32_t> pixels_;
};

} // namespace sr::gfx
