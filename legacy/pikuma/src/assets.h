#ifndef ASSETS_H
#define ASSETS_H

#include <stdint.h>

#include <SDL2/SDL.h>

#include "texture.h"
#include "mesh.h"
#include "texture_multiframe.h"
#include "texture_management.h"
#include "material_management.h"
#include "model_manager.h"

////////////////////////////////////////////////////////////////////////////////
// MISC
////////////////////////////////////////////////////////////////////////////////

typedef struct // sized sdl texture
{
    SDL_Texture *texture;
    int width;
    int height;
} SizedSDLTexture;

SizedSDLTexture *sized_sdl_texture_load(const char *filename, SDL_Renderer *renderer);
void sized_sdl_texture_free(SizedSDLTexture *sized_sdl_texture);

Mesh *mesh_load_from_obj(const char *filename);

////////////////////////////////////////////////////////////////////////////////
// Assets
////////////////////////////////////////////////////////////////////////////////

// defs for the assets, such as gba overlay, gba power light, pointer
typedef struct
{
    ModelManager *model_manager;
    MaterialManager *material_manager;
    TextureManager *texture_manager;

    MultiFrameTexture *earth_mft;
} Assets;

Assets *assets_new(void);
Assets *assets_load(void);
void assets_free(Assets *assets);

#endif // ASSETS_H