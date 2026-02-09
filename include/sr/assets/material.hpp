#pragma once

#include "sr/math/vec3.hpp"

#include <memory>
#include <string>

namespace sr::gfx {
class Texture;
}

namespace sr::assets {

struct Material {
    std::string name;
    sr::math::Vec3 base_color{1.0f, 1.0f, 1.0f};
    std::shared_ptr<sr::gfx::Texture> base_color_tex; // may be null

    bool double_sided = false;
    bool front_face_ccw = true;
};

} // namespace sr::assets
