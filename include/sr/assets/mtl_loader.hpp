#pragma once

#include "sr/assets/asset_store.hpp"
#include "sr/assets/material.hpp"

#include <filesystem>
#include <string>
#include <vector>

namespace sr::assets {

std::vector<Material> load_mtl(const std::filesystem::path& path, AssetStore& store);

} // namespace sr::assets
