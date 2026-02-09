#pragma once

#include "sr/gfx/texture.hpp"

#include <SDL2/SDL.h>

#include <memory>
#include <string>
#include <unordered_map>

namespace sr::assets {

class AssetStore {
  public:
    explicit AssetStore(SDL_Renderer* renderer) : renderer_(renderer) {}

    std::shared_ptr<sr::gfx::Texture> get_texture(const std::string& path);

  private:
    SDL_Renderer* renderer_ = nullptr;
    std::unordered_map<std::string, std::weak_ptr<sr::gfx::Texture>> textures_;
};

} // namespace sr::assets
