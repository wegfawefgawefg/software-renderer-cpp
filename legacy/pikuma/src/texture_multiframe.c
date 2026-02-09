#include "texture_multiframe.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <gif_lib.h>
#include "stb_image.h"

#include "texture.h"
#include "utils.h"

// Helper function to convert RGB to ARGB format
static uint32_t RGBToRGBA(int r, int g, int b, int a)
{
    // if the image is black return transparent
    if (r == 0 && g == 0 && b == 0)
    {
        return 0;
    }
    return ((uint32_t)r << 24) | ((uint32_t)g << 16) | ((uint32_t)b << 8) | (uint32_t)a;
}

// Helper function to get pixel color from color index
static uint32_t GetColorFromIndex(const GifFileType *gif, int colorIndex)
{
    ColorMapObject *colorMap = gif->SColorMap;
    if (colorMap == NULL || colorIndex >= colorMap->ColorCount)
    {
        // Return transparent if color map is not available or index is out of bounds
        return RGBToRGBA(0, 0, 0, 0);
    }
    GifColorType color = colorMap->Colors[colorIndex];
    return RGBToRGBA(color.Red, color.Green, color.Blue, 255);
}

// mft new
MultiFrameTexture *mft_new(int num_frames, int width, int height)
{
    MultiFrameTexture *mft = (MultiFrameTexture *)malloc(sizeof(MultiFrameTexture));
    if (mft == NULL)
    {
        return NULL;
    }

    mft->num_frames = num_frames;
    mft->current_frame = 0;
    mft->frames = (Texture **)malloc(num_frames * sizeof(Texture *));
    if (mft->frames == NULL)
    {
        free(mft);
        return NULL;
    }

    for (int i = 0; i < num_frames; i++)
    {
        mft->frames[i] = texture_new(width, height);
        if (mft->frames[i] == NULL)
        {
            mft_free(mft);
            return NULL;
        }
    }

    return mft;
}

// mft free
void mft_free(MultiFrameTexture *mft)
{
    if (mft == NULL)
    {
        return;
    }

    for (int i = 0; i < mft->num_frames; i++)
    {
        texture_free(mft->frames[i]);
    }

    free(mft->frames);
    free(mft);
}

// mft next frame
void mft_next_frame(MultiFrameTexture *mft)
{
    mft->current_frame = (mft->current_frame + 1) % mft->num_frames;
}

MultiFrameTexture *mft_load_from_gif(const char *path)
{
    char filename[256];
    // extract the filename from the path
    for (int i = strlen(path) - 1; i >= 0; i--)
    {
        if (path[i] == '/')
        {
            strncpy(filename, path + i + 1, 256);
            break;
        }
    }

    int error = 0;
    GifFileType *gif = DGifOpenFileName(path, &error);
    if (gif == NULL)
    {
        fprintf(stderr, "Failed to open GIF file %s: %s\n", filename, GifErrorString(error));
        return NULL;
    }

    if (DGifSlurp(gif) == GIF_ERROR)
    {
        fprintf(stderr, "Failed to read GIF file %s: %s\n", filename, GifErrorString(gif->Error));
        DGifCloseFile(gif, &error);
        return NULL;
    }

    int num_frames = gif->ImageCount;
    if (num_frames == 0)
    {
        fprintf(stderr, "No frames found in GIF file %s\n", filename);
        DGifCloseFile(gif, &error);
        return NULL;
    }

    // Assuming all frames have the same dimensions as the logical screen
    int width = gif->SWidth;
    int height = gif->SHeight;

    MultiFrameTexture *mft = mft_new(num_frames, width, height);
    if (mft == NULL)
    {
        fprintf(stderr, "Failed to allocate MultiFrameTexture\n");
        DGifCloseFile(gif, &error);
        return NULL;
    }

    // Initialize a base frame buffer to handle disposal methods
    Texture *base_buffer = texture_new(width, height);
    if (base_buffer == NULL)
    {
        fprintf(stderr, "Failed to allocate base Texture\n");
        mft_free(mft);
        DGifCloseFile(gif, &error);
        return NULL;
    }
    texture_clear(base_buffer);

    // Iterate over each frame
    for (int i = 0; i < num_frames; i++)
    {
        SavedImage *frame = &gif->SavedImages[i];
        GifImageDesc *imageDesc = &frame->ImageDesc;

        // Handle frame position
        int frameLeft = imageDesc->Left;
        int frameTop = imageDesc->Top;
        int frameWidth = imageDesc->Width;
        int frameHeight = imageDesc->Height;

        // Create a temporary buffer to hold the frame
        Texture *temp_buffer = texture_new(width, height);
        if (temp_buffer == NULL)
        {
            fprintf(stderr, "Failed to allocate temp Texture for frame %d\n", i);
            texture_free(base_buffer);
            mft_free(mft);
            DGifCloseFile(gif, &error);
            return NULL;
        }

        // Copy the base buffer to temp_buffer
        for (int y = 0; y < height; y++)
        {
            for (int x = 0; x < width; x++)
            {
                uint32_t color = texture_get(base_buffer, x, y);
                texture_set(temp_buffer, x, y, color);
            }
        }

        // Get the color map for this frame
        ColorMapObject *colorMap = frame->ImageDesc.ColorMap ? frame->ImageDesc.ColorMap : gif->SColorMap;
        if (colorMap == NULL)
        {
            fprintf(stderr, "No color map found for frame %d\n", i);
            texture_free(temp_buffer);
            texture_free(base_buffer);
            mft_free(mft);
            DGifCloseFile(gif, &error);
            return NULL;
        }

        // Iterate over each pixel in the frame
        for (int y = 0; y < frameHeight; y++)
        {
            for (int x = 0; x < frameWidth; x++)
            {
                int pixelIndex = y * frameWidth + x;
                GifByteType colorIndex = frame->RasterBits[pixelIndex];

                // Handle transparency if present
                uint32_t color = GetColorFromIndex(gif, colorIndex);

                // Check if this pixel is transparent
                // This example assumes that the transparent color index is set in the Graphics Control Extension
                // You may need to parse the Graphics Control Extension to get the transparent index
                // For simplicity, we'll skip handling transparency here

                texture_set(temp_buffer, frameLeft + x, frameTop + y, color);
            }
        }

        // Add the temp_buffer to MultiFrameTexture
        mft->frames[i] = temp_buffer;

        // Update the base buffer based on the disposal method
        // For simplicity, we'll assume disposal method 1 (do not dispose)
        // Handling all disposal methods requires more complex logic
        // You can refer to the GIF89a specification for details

        // In this example, we overwrite the base buffer with the current frame
        for (int y = 0; y < height; y++)
        {
            for (int x = 0; x < width; x++)
            {
                uint32_t color = texture_get(temp_buffer, x, y);
                texture_set(base_buffer, x, y, color);
            }
        }
    }

    texture_free(base_buffer);
    DGifCloseFile(gif, &error);
    return mft;
}
