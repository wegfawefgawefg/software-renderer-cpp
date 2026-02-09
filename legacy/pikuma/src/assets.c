#define _DEFAULT_SOURCE // Ensure feature test macros are set

#include "assets.h"

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>   // For directory traversal
#include <sys/stat.h> // For file information
#include <errno.h>    // For error handling

#include "utils.h"
#include "texture.h"

// sdl import
#include <SDL2/SDL_image.h>

SizedSDLTexture *sized_sdl_texture_load(const char *filename, SDL_Renderer *renderer)
{
    char path[512];
    snprintf(path, sizeof(path), "./assets/sdl_textures/%s", filename);

    SDL_Surface *surface = IMG_Load(path);
    if (!surface)
    {
        printf("Failed to load image: %s\n", IMG_GetError());
        return NULL;
    }

    SDL_Texture *texture = SDL_CreateTextureFromSurface(renderer, surface);
    if (!texture)
    {
        printf("Failed to create texture: %s\n", SDL_GetError());
        SDL_FreeSurface(surface);
        return NULL;
    }

    SizedSDLTexture *sized_sdl_texture = (SizedSDLTexture *)malloc(sizeof(SizedSDLTexture));
    sized_sdl_texture->texture = texture;
    sized_sdl_texture->width = surface->w;
    sized_sdl_texture->height = surface->h;

    SDL_FreeSurface(surface);

    return sized_sdl_texture;
}

void sized_sdl_texture_free(SizedSDLTexture *sized_sdl_texture)
{
    SDL_DestroyTexture(sized_sdl_texture->texture);
    free(sized_sdl_texture);
}

#include <stdbool.h>

// Helper function to append formatted error messages to a buffer
static void append_error(char *buffer, size_t size, size_t *offset, const char *format, ...)
{
    if (*offset >= size - 1)
        return; // Buffer is full

    va_list args;
    va_start(args, format);
    int written = vsnprintf(buffer + *offset, size - *offset, format, args);
    va_end(args);

    if (written > 0)
    {
        if ((size_t)written < size - *offset)
            *offset += written;
        else
            *offset = size - 1; // Prevent overflow
    }
}

Assets *assets_new(void)
{
    Assets *assets = (Assets *)calloc(1, sizeof(Assets));
    if (!assets)
    {
        fprintf(stderr, "Failed to allocate memory for assets\n");
        return NULL;
    }

    // set all your pointers to null

    assets->model_manager = NULL;
    assets->material_manager = NULL;
    assets->texture_manager = NULL;
    assets->earth_mft = NULL;

    return assets;
}

typedef struct
{
    bool success;
    const char *error_message;
} LoadResult;

Assets *assets_load(void)
{
    Assets *assets = (Assets *)calloc(1, sizeof(Assets));
    if (!assets)
    {
        fprintf(stderr, "Failed to allocate memory for assets\n");
        return NULL;
    }

    // Buffer to accumulate error messages
    char error_buffer[1024] = {0};
    size_t error_offset = 0;
    bool failed = false;

    const char *path = "./assets/animated_textures/earth.gif";
    assets->earth_mft = mft_load_from_gif(path);
    if (!assets->earth_mft)
    {
        append_error(error_buffer, sizeof(error_buffer), &error_offset, "Failed to load %s\n", path);
        failed = true;
    }
    printf(
        "\033[0;32mLoaded MultiFrameTexture: \033[0mfile: %s, num_frames: %d, width: %d, height: %d\n",
        path,
        assets->earth_mft->num_frames,
        assets->earth_mft->frames[0]->width,
        assets->earth_mft->frames[0]->height);

    // Check if any asset failed to load
    if (failed)
    {
        fprintf(stderr, "%s", error_buffer);
        assets_free(assets);
        return NULL;
    }

    // Load all materials from the specified directory
    const char *material_directory = "./assets/materials/";
    MaterialManager *material_manager = material_manager_load_from_directory(material_directory);
    if (!material_manager)
    {
        fprintf(stderr, "Failed to load materials from directory: %s\n", material_directory);
    }
    assets->material_manager = material_manager;
    material_manager_print(material_manager);

    // Load all PNG textures from the specified directory
    const char *texture_directory = "./assets/textures/";
    TextureManager *texture_manager = texture_manager_load_from_directory(texture_directory);
    if (!texture_manager)
    {
        fprintf(stderr, "Failed to load textures from directory: %s\n", texture_directory);
        // Depending on your application's requirements, you might choose to exit or continue
    }
    assets->texture_manager = texture_manager;
    texture_manager_print(texture_manager);

    // Load all 3d model related files from the specified directory
    const char *model_directory = "./assets/models/";
    ModelManager *model_manager = model_manager_load_from_directory(model_directory);
    if (!model_manager)
    {
        fprintf(stderr, "Failed to load models from directory: %s\n", model_directory);
    }
    assets->model_manager = model_manager;
    model_manager_print(model_manager);

    // All assets loaded successfully
    return assets;
}

void assets_free(Assets *assets)
{
    if (!assets)
    {
        return;
    }

    model_manager_free(assets->model_manager);
    material_manager_free(assets->material_manager);
    texture_manager_free(assets->texture_manager);

    mft_free(assets->earth_mft);

    free(assets);
}

void replace_extension_with_col(const char *filename, char *col_filename, size_t size)
{
    // Find the last occurrence of '.' in the filename
    const char *dot = strrchr(filename, '.');
    if (dot)
    {
        // Calculate the length up to the last '.'
        size_t basename_length = dot - filename;
        // Ensure we don't exceed the buffer size
        if (basename_length < size)
        {
            strncpy(col_filename, filename, basename_length);
            col_filename[basename_length] = '\0'; // Null-terminate
        }
        else
        {
            // If basename is too long, truncate
            strncpy(col_filename, filename, size - 1);
            col_filename[size - 1] = '\0';
        }
    }
    else
    {
        // If there's no '.', copy the entire filename
        strncpy(col_filename, filename, size - 1);
        col_filename[size - 1] = '\0';
    }

    // Append ".col"
    strncat(col_filename, ".col", size - strlen(col_filename) - 1);
}
