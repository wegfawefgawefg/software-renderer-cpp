#pragma once

#include "sr/assets/material.hpp"
#include "sr/assets/mesh.hpp"
#include "sr/math/vec3.hpp"

#include <cstdint>
#include <vector>

namespace sr::assets {

struct Primitive {
  uint32_t index_offset = 0;
  uint32_t index_count = 0;
  uint32_t material_index = 0;
};

struct Model {
  Mesh mesh;
  std::vector<Material> materials;
  std::vector<Primitive> primitives;

  // Bounds in model space.
  sr::math::Vec3 bounds_center{0.0f, 0.0f, 0.0f};
  float bounds_radius = 1.0f;
};

}  // namespace sr::assets

