#pragma once

#include "sr/assets/asset_store.hpp"
#include "sr/assets/model.hpp"

#include <filesystem>

namespace sr::assets {

struct ObjModelLoadOptions {
    bool flip_v = true;
    // Winding / culling conventions:
    // Many OBJ exports are inconsistent. Default to CW front for now because
    // Peach's Castle (our main asset) is authored that way.
    bool front_face_ccw = false;
    bool double_sided = false;
};

Model load_obj_model(const std::filesystem::path& path, AssetStore& store,
                     const ObjModelLoadOptions& opt = {});

} // namespace sr::assets
