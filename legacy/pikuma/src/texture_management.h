#ifndef TEXTURE_MANAGEMENT_H
#define TEXTURE_MANAGEMENT_H

#include <stdint.h>
#include <SDL2/SDL.h>
#include "vec2.h"
#include "texture.h"

// Structure to hold individual texture properties
typedef struct
{
    char *path;       // Full path to the texture file
    char *filename;   // Filename of the texture
    Texture *texture; // Loaded Texture
} TextureManagerEntry;

// we have a lot of textures, we have to put them somewhere
typedef struct
{
    TextureManagerEntry *entries; // Dynamic array of texture entries
    int num_entries;              // Current number of textures loaded
} TextureManager;

TextureManager *texture_manager_load_from_directory(const char *directory_path);
void texture_manager_free(TextureManager *textures);
void texture_manager_print(TextureManager *textures);

// gets a texture by its filename, or NULL if not found
Texture *texture_manager_get(TextureManager *textures, const char *filename);

#endif // TEXTURE_MANAGEMENT_H
