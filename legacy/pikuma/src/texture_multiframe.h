#ifndef TEXTURE_MULTIFRAME_H
#define TEXTURE_MULTIFRAME_H

#include <stdint.h>

#include <SDL2/SDL.h>

#include "vec2.h"
#include "texture.h"

// multiframe texture
typedef struct
{
    int num_frames;
    int current_frame;
    Texture **frames;
} MultiFrameTexture;

MultiFrameTexture *mft_new(int num_frames, int width, int height);
void mft_free(MultiFrameTexture *mft);
void mft_next_frame(MultiFrameTexture *mft);
MultiFrameTexture *mft_load_from_gif(const char *path);

#endif