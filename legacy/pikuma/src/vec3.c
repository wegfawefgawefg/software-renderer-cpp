#include "vec3.h"
#include <math.h>

// Vec3 implementations (float)
Vec3 vec3_create(float x, float y, float z)
{
    return (Vec3){x, y, z};
}

Vec3 vec3_add(Vec3 a, Vec3 b)
{
    return (Vec3){a.x + b.x, a.y + b.y, a.z + b.z};
}

Vec3 vec3_sub(Vec3 a, Vec3 b)
{
    return (Vec3){a.x - b.x, a.y - b.y, a.z - b.z};
}

Vec3 vec3_mul(Vec3 v, float scalar)
{
    return (Vec3){v.x * scalar, v.y * scalar, v.z * scalar};
}

Vec3 vec3_fmul(Vec3 v, float scalar)
{
    return (Vec3){v.x * scalar, v.y * scalar, v.z * scalar};
}

Vec3 vec3_div(Vec3 v, float scalar)
{
    return (Vec3){v.x / scalar, v.y / scalar, v.z / scalar};
}

float vec3_dot(Vec3 a, Vec3 b)
{
    return a.x * b.x + a.y * b.y + a.z * b.z;
}

float vec3_length(Vec3 v)
{
    return sqrtf(v.x * v.x + v.y * v.y + v.z * v.z);
}

float vec3_distance(Vec3 a, Vec3 b)
{
    return vec3_length(vec3_sub(a, b));
}

Vec3 vec3_cross(Vec3 a, Vec3 b)
{
    return (Vec3){
        a.y * b.z - a.z * b.y,
        a.z * b.x - a.x * b.z,
        a.x * b.y - a.y * b.x};
}

Vec3 vec3_normalize(Vec3 v)
{
    float len = vec3_length(v);
    if (len != 0)
    {
        return vec3_div(v, len);
    }
    return v;
}

Vec3 vec3_rotate_point(Vec3 point, Vec3 center, Vec3 rotation)
{
    Vec3 result = point;

    // Translate point to origin
    result.x -= center.x;
    result.y -= center.y;
    result.z -= center.z;

    // Rotate around X axis
    float cosX = cos(rotation.x);
    float sinX = sin(rotation.x);
    float y = result.y;
    float z = result.z;
    result.y = y * cosX - z * sinX;
    result.z = z * cosX + y * sinX;

    // Rotate around Y axis
    float cosY = cos(rotation.y);
    float sinY = sin(rotation.y);
    float x = result.x;
    z = result.z;
    result.x = x * cosY + z * sinY;
    result.z = z * cosY - x * sinY;

    // Rotate around Z axis
    float cosZ = cos(rotation.z);
    float sinZ = sin(rotation.z);
    x = result.x;
    y = result.y;
    result.x = x * cosZ - y * sinZ;
    result.y = y * cosZ + x * sinZ;

    // Translate point back
    result.x += center.x;
    result.y += center.y;
    result.z += center.z;

    return result;
}

// IVec3 implementations (integer)
IVec3 ivec3_create(int x, int y, int z)
{
    return (IVec3){x, y, z};
}

IVec3 ivec3_add(IVec3 a, IVec3 b)
{
    return (IVec3){a.x + b.x, a.y + b.y, a.z + b.z};
}

IVec3 ivec3_sub(IVec3 a, IVec3 b)
{
    return (IVec3){a.x - b.x, a.y - b.y, a.z - b.z};
}

IVec3 ivec3_mul(IVec3 v, int scalar)
{
    return (IVec3){v.x * scalar, v.y * scalar, v.z * scalar};
}

IVec3 ivec3_div(IVec3 v, int scalar)
{
    return (IVec3){v.x / scalar, v.y / scalar, v.z / scalar};
}

int ivec3_dot(IVec3 a, IVec3 b)
{
    return a.x * b.x + a.y * b.y + a.z * b.z;
}

IVec3 ivec3_cross(IVec3 a, IVec3 b)
{
    return (IVec3){
        a.y * b.z - a.z * b.y,
        a.z * b.x - a.x * b.z,
        a.x * b.y - a.y * b.x};
}