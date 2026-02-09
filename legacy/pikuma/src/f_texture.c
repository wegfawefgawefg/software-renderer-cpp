#include "f_texture.h"

#include <stdlib.h>
#include <float.h>

FTexture *f_texture_new(int width, int height)
{
    FTexture *ft = malloc(sizeof(FTexture));
    ft->width = width;
    ft->height = height;
    ft->data = malloc(sizeof(float) * width * height);
    return ft;
}
void f_texture_free(FTexture *ft)
{
    free(ft->data);
    free(ft);
}

void f_texture_zero(FTexture *ft)
{
    for (int i = 0; i < ft->width * ft->height; i++)
    {
        ft->data[i] = 0.0f;
    }
}

void f_texture_fill_float_max(FTexture *ft)
{
    for (int i = 0; i < ft->width * ft->height; i++)
    {
        ft->data[i] = FLT_MAX;
    }
}

void f_texture_set(FTexture *ft, int x, int y, float value)
{
    if (x < 0 || x >= ft->width || y < 0 || y >= ft->height)
    {
        return;
    }
    ft->data[y * ft->width + x] = value;
}
float f_texture_get(FTexture *ft, int x, int y)
{
    if (x < 0 || x >= ft->width || y < 0 || y >= ft->height)
    {
        return 0.0f;
    }
    return ft->data[y * ft->width + x];
}