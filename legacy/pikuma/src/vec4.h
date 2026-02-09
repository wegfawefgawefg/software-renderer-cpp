#ifndef VEC4_H
#define VEC4_H

typedef struct
{
    float x;
    float y;
    float z;
    float w;
} Vec4;

Vec4 vec4_create(float x, float y, float z, float w);
Vec4 vec4_mul(Vec4 v, Vec4 s);

#endif