#pragma once

#include "sr/math/vec3.hpp"

#include <memory>
#include <string>

namespace sr::gfx {
class Texture;
}

namespace sr::assets {

enum class AlphaMode : uint8_t {
    Opaque = 0,
    Mask = 1,  // alpha test / cutout
    Blend = 2, // alpha blend
};

struct Material {
    std::string name;
    sr::math::Vec3 base_color{1.0f, 1.0f, 1.0f};
    std::shared_ptr<sr::gfx::Texture> base_color_tex; // may be null

    AlphaMode alpha_mode = AlphaMode::Opaque;
    float alpha_cutoff = 0.5f; // Only used for Mask.

    bool double_sided = false;
    bool front_face_ccw = true;
};

} // namespace sr::assets
