#include "vec4.h"

Vec4 vec4_create(float x, float y, float z, float w)
{
    Vec4 v = {x, y, z, w};
    return v;
}

Vec4 vec4_mul(Vec4 v, Vec4 s)
{
    Vec4 result = {v.x * s.x, v.y * s.y, v.z * s.z, v.w * s.w};
    return result;
}