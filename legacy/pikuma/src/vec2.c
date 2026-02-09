#include "vec2.h"
#include <math.h>

// Vec2 implementations (float)
Vec2 vec2_create(float x, float y)
{
    return (Vec2){x, y};
}

Vec2 vec2_add(Vec2 a, Vec2 b)
{
    return (Vec2){a.x + b.x, a.y + b.y};
}

Vec2 vec2_sub(Vec2 a, Vec2 b)
{
    return (Vec2){a.x - b.x, a.y - b.y};
}

Vec2 vec2_mul(Vec2 v, Vec2 w)
{
    return (Vec2){v.x * w.x, v.y * w.y};
}

Vec2 vec2_fmul(Vec2 v, float scalar)
{
    return (Vec2){v.x * scalar, v.y * scalar};
}

float vec2_dot(Vec2 a, Vec2 b)
{
    return a.x * b.x + a.y * b.y;
}

float vec2_length(Vec2 v)
{
    return sqrtf(v.x * v.x + v.y * v.y);
}

float vec2_distance(Vec2 a, Vec2 b)
{
    return vec2_length(vec2_sub(a, b));
}

Vec2 vec2_normalize(Vec2 v)
{
    float len = vec2_length(v);
    if (len != 0)
    {
        return vec2_fmul(v, 1.0f / len);
    }
    return v;
}

Vec2 vec2_rotate_point_around_pivot(Vec2 point, Vec2 pivot, float degrees)
{
    // Convert degrees to radians
    float radians = degrees * (M_PI / 180.0f);
    float cos_r = cosf(radians);
    float sin_r = sinf(radians);

    // Translate point to origin
    float translated_x = point.x - pivot.x;
    float translated_y = point.y - pivot.y;

    // Rotate the point
    float rotated_x = translated_x * cos_r - translated_y * sin_r;
    float rotated_y = translated_x * sin_r + translated_y * cos_r;

    // Translate back
    Vec2 result;
    result.x = rotated_x + pivot.x;
    result.y = rotated_y + pivot.y;

    return result;
}

// IVec2 implementations (integer)
IVec2 ivec2_create(int x, int y)
{
    return (IVec2){x, y};
}

IVec2 vec2_to_ivec2(Vec2 v)
{
    return (IVec2){(int)v.x, (int)v.y};
}

Vec2 ivec2_to_vec2(IVec2 v)
{
    return (Vec2){(float)v.x, (float)v.y};
}

IVec2 ivec2_add(IVec2 a, IVec2 b)
{
    return (IVec2){a.x + b.x, a.y + b.y};
}

IVec2 ivec2_sub(IVec2 a, IVec2 b)
{
    return (IVec2){a.x - b.x, a.y - b.y};
}

IVec2 ivec2_fmul(IVec2 v, int scalar)
{
    return (IVec2){v.x * scalar, v.y * scalar};
}

int ivec2_dot(IVec2 a, IVec2 b)
{
    return a.x * b.x + a.y * b.y;
}
