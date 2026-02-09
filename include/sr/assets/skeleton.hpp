#pragma once

#include "sr/math/mat4.hpp"
#include "sr/math/trs.hpp"

#include <cstdint>
#include <string>
#include <vector>

namespace sr::assets {

struct Joint {
    std::string name;
    int parent = -1; // index into Skeleton::joints, -1 = root
    sr::math::Trs rest_local;

    // Inverse bind matrix in mesh-geometry space.
    // For joints that are not part of the skin, this is identity.
    sr::math::Mat4 inv_bind = sr::math::Mat4::identity();
};

struct Skeleton {
    std::vector<Joint> joints;
};

} // namespace sr::assets

