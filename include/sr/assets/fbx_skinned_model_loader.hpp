#pragma once

#include "sr/assets/animation.hpp"
#include "sr/assets/asset_store.hpp"
#include "sr/assets/skinned_model.hpp"

#include <filesystem>
#include <string>

namespace sr::assets {

struct FbxSkinnedModelLoadOptions {
    bool flip_v = true;
    bool front_face_ccw = true;
    bool double_sided = false;

    // Optional: if non-empty, force all primitives to use this texture.
    std::string override_diffuse_texture;
};

SkinnedModel load_fbx_skinned_model(const std::filesystem::path& path, AssetStore& store,
                                    const FbxSkinnedModelLoadOptions& opt = {});

// Loads an animation clip from an FBX file and bakes it to uniform samples.
// Joint mapping is done by joint name against `skel`.
AnimationClip load_fbx_animation_clip(const std::filesystem::path& path, const Skeleton& skel,
                                      const std::string& clip_name, float sample_rate = 30.0f);

} // namespace sr::assets

