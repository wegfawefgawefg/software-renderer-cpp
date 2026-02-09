#pragma once

#include <algorithm>
#include <limits>
#include <vector>

namespace sr::gfx {

class DepthBuffer {
  public:
    DepthBuffer(int w, int h)
        : width_(w), height_(h), z_(w * h, std::numeric_limits<float>::infinity()) {}

    int width() const { return width_; }
    int height() const { return height_; }

    void clear(float v = std::numeric_limits<float>::infinity()) {
        std::fill(z_.begin(), z_.end(), v);
    }

    float get(int x, int y) const { return z_[y * width_ + x]; }
    void set(int x, int y, float v) { z_[y * width_ + x] = v; }

  private:
    int width_ = 0;
    int height_ = 0;
    std::vector<float> z_;
};

} // namespace sr::gfx
