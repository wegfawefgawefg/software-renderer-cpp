#ifndef VEC3_H
#define VEC3_H

typedef struct
{
    float x;
    float y;
    float z;
} Vec3;

typedef struct
{
    int x;
    int y;
    int z;
} IVec3;

// Vec3 operations (float)
Vec3 vec3_create(float x, float y, float z);
Vec3 vec3_add(Vec3 a, Vec3 b);
Vec3 vec3_sub(Vec3 a, Vec3 b);
Vec3 vec3_mul(Vec3 v, float scalar);
Vec3 vec3_fmul(Vec3 v, float scalar);
Vec3 vec3_div(Vec3 v, float scalar);
float vec3_dot(Vec3 a, Vec3 b);
Vec3 vec3_cross(Vec3 a, Vec3 b);
float vec3_length(Vec3 v);
float vec3_distance(Vec3 a, Vec3 b);
Vec3 vec3_normalize(Vec3 v);
Vec3 vec3_rotate_point(Vec3 point, Vec3 center, Vec3 rotation);

// IVec3 operations (integer)
IVec3 ivec3_create(int x, int y, int z);
IVec3 ivec3_add(IVec3 a, IVec3 b);
IVec3 ivec3_sub(IVec3 a, IVec3 b);
IVec3 ivec3_mul(IVec3 v, int scalar);
IVec3 ivec3_div(IVec3 v, int scalar);
int ivec3_dot(IVec3 a, IVec3 b);
IVec3 ivec3_cross(IVec3 a, IVec3 b);

#endif // VEC3_H