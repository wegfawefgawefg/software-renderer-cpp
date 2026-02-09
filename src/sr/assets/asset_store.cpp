#include "sr/assets/asset_store.hpp"

#include "sr/assets/texture_loader.hpp"

namespace sr::assets {

std::shared_ptr<sr::gfx::Texture> AssetStore::get_texture(const std::string& path) {
  auto it = textures_.find(path);
  if (it != textures_.end()) {
    if (auto sp = it->second.lock()) return sp;
  }

  auto tex = std::make_shared<sr::gfx::Texture>(load_texture_rgba8888(renderer_, path));
  textures_[path] = tex;
  return tex;
}

}  // namespace sr::assets

