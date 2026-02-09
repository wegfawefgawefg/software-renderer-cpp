#pragma once

#include "sr/assets/asset_store.hpp"
#include "sr/assets/model.hpp"

#include <filesystem>

namespace sr::assets {

struct GltfModelLoadOptions {
    bool flip_v = false;
    bool front_face_ccw = true;
    bool double_sided = false;
};

Model load_gltf_model(const std::filesystem::path& path, AssetStore& store,
                      const GltfModelLoadOptions& opt = {});

} // namespace sr::assets
