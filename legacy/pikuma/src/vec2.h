#ifndef VEC2_H
#define VEC2_H

typedef struct
{
    float x;
    float y;
} Vec2;

typedef struct
{
    int x;
    int y;
} IVec2;

// Vec2 operations (float)
Vec2 vec2_create(float x, float y);
Vec2 vec2_add(Vec2 a, Vec2 b);
Vec2 vec2_sub(Vec2 a, Vec2 b);
Vec2 vec2_mul(Vec2 a, Vec2 b);
Vec2 vec2_fmul(Vec2 v, float scalar);
float vec2_dot(Vec2 a, Vec2 b);
float vec2_length(Vec2 v);
float vec2_distance(Vec2 a, Vec2 b);
Vec2 vec2_normalize(Vec2 v);
Vec2 vec2_rotate_point_around_pivot(Vec2 point, Vec2 pivot, float degrees);
IVec2 vec2_to_ivec2(Vec2 v);

// IVec2 operations (integer)
IVec2 ivec2_create(int x, int y);
Vec2 ivec2_to_vec2(IVec2 v);
IVec2 ivec2_add(IVec2 a, IVec2 b);
IVec2 ivec2_sub(IVec2 a, IVec2 b);
IVec2 ivec2_fmul(IVec2 v, int scalar);
int ivec2_dot(IVec2 a, IVec2 b);

#endif // VEC2_H