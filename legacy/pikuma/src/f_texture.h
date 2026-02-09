#ifndef F_TEXTURE
#define F_TEXTURE

typedef struct
{
    int width;
    int height;
    float *data;
} FTexture;

FTexture *f_texture_new(int width, int height);
void f_texture_free(FTexture *ft);
void f_texture_zero(FTexture *ft);
void f_texture_fill_float_max(FTexture *ft);

// set and get
void f_texture_set(FTexture *ft, int x, int y, float value);
float f_texture_get(FTexture *ft, int x, int y);

#endif