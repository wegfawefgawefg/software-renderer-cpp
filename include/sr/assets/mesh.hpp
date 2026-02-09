#pragma once

#include "sr/math/vec2.hpp"
#include "sr/math/vec3.hpp"

#include <cstdint>
#include <vector>

namespace sr::assets {

struct Mesh {
  std::vector<sr::math::Vec3> positions;
  std::vector<sr::math::Vec2> uvs;  // optional; empty = none
  std::vector<uint32_t> indices;    // triangle list, 3*n
};

}  // namespace sr::assets

