#pragma once

#include "sr/assets/model.hpp"
#include "sr/math/mat4.hpp"
#include "sr/render/renderer.hpp"

#include <memory>
#include <vector>

namespace sr::scene {

struct Entity {
    std::shared_ptr<sr::assets::Model> model;
    sr::math::Mat4 transform = sr::math::Mat4::identity();
};

struct Scene {
    sr::render::Camera camera;
    std::vector<Entity> entities;
};

} // namespace sr::scene
