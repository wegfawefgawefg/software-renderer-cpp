#pragma once

#include "sr/assets/mesh.hpp"

#include <string>

namespace sr::assets {

Mesh load_obj_minimal(const std::string& path, bool flip_v = true);

} // namespace sr::assets
