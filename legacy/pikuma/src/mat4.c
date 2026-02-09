#include "mat4.h"
#include <math.h>
#include <stdio.h>
#include <string.h>
#include "vec4.h"

#include "utils.h"

// Utility function to set matrix to identity
Mat4 mat4_identity(void)
{
    Mat4 result = {0};
    for (int i = 0; i < 4; i++)
    {
        result.m[i][i] = 1.0f;
    }
    return result;
}

// Create a matrix with specified elements
Mat4 mat4_create(
    float m00, float m01, float m02, float m03,
    float m10, float m11, float m12, float m13,
    float m20, float m21, float m22, float m23,
    float m30, float m31, float m32, float m33)
{
    Mat4 result = {
        .m = {
            {m00, m01, m02, m03},
            {m10, m11, m12, m13},
            {m20, m21, m22, m23},
            {m30, m31, m32, m33}}};
    return result;
}

// Copy a matrix
Mat4 mat4_copy(Mat4 src)
{
    Mat4 dest;
    memcpy(dest.m, src.m, sizeof(float) * 16);
    return dest;
}

// Multiplies two 4x4 matrices
Mat4 mat4_multiply(Mat4 a, Mat4 b)
{
    Mat4 result = {0};

    for (int row = 0; row < 4; row++)
    {
        for (int col = 0; col < 4; col++)
        {
            for (int k = 0; k < 4; k++)
            {
                result.m[row][col] += a.m[row][k] * b.m[k][col];
            }
        }
    }

    return result;
}

// Multiply a matrix with a Vec3 (assuming w = 1)
Vec3 mat4_multiply_vec3(Mat4 m, Vec3 v)
{
    float x = m.m[0][0] * v.x + m.m[0][1] * v.y + m.m[0][2] * v.z + m.m[0][3];
    float y = m.m[1][0] * v.x + m.m[1][1] * v.y + m.m[1][2] * v.z + m.m[1][3];
    float z = m.m[2][0] * v.x + m.m[2][1] * v.y + m.m[2][2] * v.z + m.m[2][3];
    float w = m.m[3][0] * v.x + m.m[3][1] * v.y + m.m[3][2] * v.z + m.m[3][3];

    if (w != 0.0f && w != 1.0f)
    {
        x /= w;
        y /= w;
        z /= w;
    }

    Vec3 result = {x, y, z};
    return result;
}

// Function to multiply a Mat4 with a Vec4
Vec4 mat4_multiply_vec4(const Mat4 m, const Vec4 v)
{
    Vec4 result;

    result.x = m.m[0][0] * v.x + m.m[0][1] * v.y + m.m[0][2] * v.z + m.m[0][3] * v.w;
    result.y = m.m[1][0] * v.x + m.m[1][1] * v.y + m.m[1][2] * v.z + m.m[1][3] * v.w;
    result.z = m.m[2][0] * v.x + m.m[2][1] * v.y + m.m[2][2] * v.z + m.m[2][3] * v.w;
    result.w = m.m[3][0] * v.x + m.m[3][1] * v.y + m.m[3][2] * v.z + m.m[3][3] * v.w;

    return result;
}

// Creates a translation matrix given a translation vector
Mat4 mat4_translate(Vec3 translation)
{
    Mat4 result = mat4_identity();
    result.m[0][3] = translation.x;
    result.m[1][3] = translation.y;
    result.m[2][3] = translation.z;
    return result;
}

// Create a scaling matrix
Mat4 mat4_scale(Vec3 v)
{
    Mat4 result = mat4_identity();
    result.m[0][0] = v.x;
    result.m[1][1] = v.y;
    result.m[2][2] = v.z;
    return result;
}

// Create a rotation matrix around the X-axis
Mat4 mat4_rotate_x(float angle_rad)
{
    Mat4 result = mat4_identity();
    float c = cosf(angle_rad);
    float s = sinf(angle_rad);
    result.m[1][1] = c;
    result.m[1][2] = -s;
    result.m[2][1] = s;
    result.m[2][2] = c;
    return result;
}

// Create a rotation matrix around the Y-axis
Mat4 mat4_rotate_y(float angle_rad)
{
    Mat4 result = mat4_identity();
    float c = cosf(angle_rad);
    float s = sinf(angle_rad);
    result.m[0][0] = c;
    result.m[0][2] = s;
    result.m[2][0] = -s;
    result.m[2][2] = c;
    return result;
}

// Create a rotation matrix around the Z-axis
Mat4 mat4_rotate_z(float angle_rad)
{
    Mat4 result = mat4_identity();
    float c = cosf(angle_rad);
    float s = sinf(angle_rad);
    result.m[0][0] = c;
    result.m[0][1] = -s;
    result.m[1][0] = s;
    result.m[1][1] = c;
    return result;
}

// Create a rotation matrix around an arbitrary axis
Mat4 mat4_rotate(float angle_rad, Vec3 axis)
{
    Vec3 norm = vec3_normalize(axis);
    float x = norm.x;
    float y = norm.y;
    float z = norm.z;
    float c = cosf(angle_rad);
    float s = sinf(angle_rad);
    float one_c = 1.0f - c;

    Mat4 result = mat4_identity();
    result.m[0][0] = c + x * x * one_c;
    result.m[0][1] = x * y * one_c - z * s;
    result.m[0][2] = x * z * one_c + y * s;

    result.m[1][0] = y * x * one_c + z * s;
    result.m[1][1] = c + y * y * one_c;
    result.m[1][2] = y * z * one_c - x * s;

    result.m[2][0] = z * x * one_c - y * s;
    result.m[2][1] = z * y * one_c + x * s;
    result.m[2][2] = c + z * z * one_c;

    return result;
}

// Create a perspective projection matrix
Mat4 mat4_perspective(float fov_rad, float aspect, float near, float far)
{
    Mat4 result = {0};
    float f = 1.0f / tanf(fov_rad / 2.0f);
    result.m[0][0] = f / aspect;
    result.m[1][1] = f;
    result.m[2][2] = (far + near) / (near - far);
    result.m[2][3] = (2.0f * far * near) / (near - far);
    result.m[3][2] = -1.0f;
    return result;
}

// Orthographic projection
Mat4 orthographic(float left, float right, float bottom, float top, float near, float far)
{
    Mat4 ortho = {0};

    ortho.m[0][0] = 2.0f / (right - left);
    ortho.m[1][1] = 2.0f / (top - bottom);
    ortho.m[2][2] = -2.0f / (far - near);
    ortho.m[3][3] = 1.0f;

    ortho.m[0][3] = -(right + left) / (right - left);
    ortho.m[1][3] = -(top + bottom) / (top - bottom);
    ortho.m[2][3] = -(far + near) / (far - near);

    return ortho;
}

#define PI 3.14159265358979323846

// Create isometric projection matrix
Mat4 mat4_isometric(float left, float right, float bottom, float top, float near, float far)
{
    // Define rotation angles in radians
    float angle_y = 45.0f * (PI / 180.0f);   // 45 degrees
    float angle_x = 35.264f * (PI / 180.0f); // ~35.264 degrees

    // Create rotation matrices
    Mat4 rot_y = mat4_rotate_y(angle_y);
    Mat4 rot_x = mat4_rotate_x(angle_x);

    // Combine rotations: First rotate around Y, then around X
    Mat4 rotation = mat4_multiply(rot_x, rot_y);

    // Create orthographic projection matrix
    Mat4 ortho = orthographic(left, right, bottom, top, near, far);

    // Combine orthographic projection with rotation
    Mat4 isometric = mat4_multiply(ortho, rotation);

    return isometric;
}

// Create a look-at view matrix
Mat4 mat4_look_at(Vec3 eye, Vec3 center, Vec3 up)
{
    Vec3 f = vec3_normalize(vec3_sub(center, eye));
    Vec3 s = vec3_normalize(vec3_cross(f, up));
    Vec3 u = vec3_cross(s, f);

    Mat4 result = mat4_identity();
    result.m[0][0] = s.x;
    result.m[0][1] = s.y;
    result.m[0][2] = s.z;

    result.m[1][0] = u.x;
    result.m[1][1] = u.y;
    result.m[1][2] = u.z;

    result.m[2][0] = -f.x;
    result.m[2][1] = -f.y;
    result.m[2][2] = -f.z;

    result.m[0][3] = -vec3_dot(s, eye);
    result.m[1][3] = -vec3_dot(u, eye);
    result.m[2][3] = vec3_dot(f, eye);

    return result;
}

// Transpose of a matrix
Mat4 mat4_transpose(Mat4 m)
{
    Mat4 result;
    for (int i = 0; i < 4; i++)
    {
        for (int j = 0; j < 4; j++)
        {
            result.m[i][j] = m.m[j][i];
        }
    }
    return result;
}

// Inverse of a matrix (assuming it's invertible)
Mat4 mat4_inverse(Mat4 m)
{
    Mat4 inv;
    float det;
    int i;

    inv.m[0][0] = m.m[1][1] * m.m[2][2] * m.m[3][3] -
                  m.m[1][1] * m.m[2][3] * m.m[3][2] -
                  m.m[2][1] * m.m[1][2] * m.m[3][3] +
                  m.m[2][1] * m.m[1][3] * m.m[3][2] +
                  m.m[3][1] * m.m[1][2] * m.m[2][3] -
                  m.m[3][1] * m.m[1][3] * m.m[2][2];

    inv.m[0][1] = -m.m[0][1] * m.m[2][2] * m.m[3][3] +
                  m.m[0][1] * m.m[2][3] * m.m[3][2] +
                  m.m[2][1] * m.m[0][2] * m.m[3][3] -
                  m.m[2][1] * m.m[0][3] * m.m[3][2] -
                  m.m[3][1] * m.m[0][2] * m.m[2][3] +
                  m.m[3][1] * m.m[0][3] * m.m[2][2];

    inv.m[0][2] = m.m[0][1] * m.m[1][2] * m.m[3][3] -
                  m.m[0][1] * m.m[1][3] * m.m[3][2] -
                  m.m[1][1] * m.m[0][2] * m.m[3][3] +
                  m.m[1][1] * m.m[0][3] * m.m[3][2] +
                  m.m[3][1] * m.m[0][2] * m.m[1][3] -
                  m.m[3][1] * m.m[0][3] * m.m[1][2];

    inv.m[0][3] = -m.m[0][1] * m.m[1][2] * m.m[2][3] +
                  m.m[0][1] * m.m[1][3] * m.m[2][2] +
                  m.m[1][1] * m.m[0][2] * m.m[2][3] -
                  m.m[1][1] * m.m[0][3] * m.m[2][2] -
                  m.m[2][1] * m.m[0][2] * m.m[1][3] +
                  m.m[2][1] * m.m[0][3] * m.m[1][2];

    inv.m[1][0] = -m.m[1][0] * m.m[2][2] * m.m[3][3] +
                  m.m[1][0] * m.m[2][3] * m.m[3][2] +
                  m.m[2][0] * m.m[1][2] * m.m[3][3] -
                  m.m[2][0] * m.m[1][3] * m.m[3][2] -
                  m.m[3][0] * m.m[1][2] * m.m[2][3] +
                  m.m[3][0] * m.m[1][3] * m.m[2][2];

    inv.m[1][1] = m.m[0][0] * m.m[2][2] * m.m[3][3] -
                  m.m[0][0] * m.m[2][3] * m.m[3][2] -
                  m.m[2][0] * m.m[0][2] * m.m[3][3] +
                  m.m[2][0] * m.m[0][3] * m.m[3][2] +
                  m.m[3][0] * m.m[0][2] * m.m[2][3] -
                  m.m[3][0] * m.m[0][3] * m.m[2][2];

    inv.m[1][2] = -m.m[0][0] * m.m[1][2] * m.m[3][3] +
                  m.m[0][0] * m.m[1][3] * m.m[3][2] +
                  m.m[1][0] * m.m[0][2] * m.m[3][3] -
                  m.m[1][0] * m.m[0][3] * m.m[3][2] -
                  m.m[3][0] * m.m[0][2] * m.m[1][3] +
                  m.m[3][0] * m.m[0][3] * m.m[1][2];

    inv.m[1][3] = m.m[0][0] * m.m[1][2] * m.m[2][3] -
                  m.m[0][0] * m.m[1][3] * m.m[2][2] -
                  m.m[1][0] * m.m[0][2] * m.m[2][3] +
                  m.m[1][0] * m.m[0][3] * m.m[2][2] +
                  m.m[2][0] * m.m[0][2] * m.m[1][3] -
                  m.m[2][0] * m.m[0][3] * m.m[1][2];

    inv.m[2][0] = m.m[1][0] * m.m[2][1] * m.m[3][3] -
                  m.m[1][0] * m.m[2][3] * m.m[3][1] -
                  m.m[2][0] * m.m[1][1] * m.m[3][3] +
                  m.m[2][0] * m.m[1][3] * m.m[3][1] +
                  m.m[3][0] * m.m[1][1] * m.m[2][3] -
                  m.m[3][0] * m.m[1][3] * m.m[2][1];

    inv.m[2][1] = -m.m[0][0] * m.m[2][1] * m.m[3][3] +
                  m.m[0][0] * m.m[2][3] * m.m[3][1] +
                  m.m[2][0] * m.m[0][1] * m.m[3][3] -
                  m.m[2][0] * m.m[0][3] * m.m[3][1] -
                  m.m[3][0] * m.m[0][1] * m.m[2][3] +
                  m.m[3][0] * m.m[0][3] * m.m[2][1];

    inv.m[2][2] = m.m[0][0] * m.m[1][1] * m.m[3][3] -
                  m.m[0][0] * m.m[1][3] * m.m[3][1] -
                  m.m[1][0] * m.m[0][1] * m.m[3][3] +
                  m.m[1][0] * m.m[0][3] * m.m[3][1] +
                  m.m[3][0] * m.m[0][1] * m.m[1][3] -
                  m.m[3][0] * m.m[0][3] * m.m[1][1];

    inv.m[2][3] = -m.m[0][0] * m.m[1][1] * m.m[2][3] +
                  m.m[0][0] * m.m[1][3] * m.m[2][1] +
                  m.m[1][0] * m.m[0][1] * m.m[2][3] -
                  m.m[1][0] * m.m[0][3] * m.m[2][1] -
                  m.m[2][0] * m.m[0][1] * m.m[1][3] +
                  m.m[2][0] * m.m[0][3] * m.m[1][1];

    inv.m[3][0] = -m.m[1][0] * m.m[2][1] * m.m[3][2] +
                  m.m[1][0] * m.m[2][2] * m.m[3][1] +
                  m.m[2][0] * m.m[1][1] * m.m[3][2] -
                  m.m[2][0] * m.m[1][2] * m.m[3][1] -
                  m.m[3][0] * m.m[1][1] * m.m[2][2] +
                  m.m[3][0] * m.m[1][2] * m.m[2][1];

    inv.m[3][1] = m.m[0][0] * m.m[2][1] * m.m[3][2] -
                  m.m[0][0] * m.m[2][2] * m.m[3][1] -
                  m.m[2][0] * m.m[0][1] * m.m[3][2] +
                  m.m[2][0] * m.m[0][2] * m.m[3][1] +
                  m.m[3][0] * m.m[0][1] * m.m[2][2] -
                  m.m[3][0] * m.m[0][2] * m.m[2][1];

    inv.m[3][2] = -m.m[0][0] * m.m[1][1] * m.m[3][2] +
                  m.m[0][0] * m.m[1][2] * m.m[3][1] +
                  m.m[1][0] * m.m[0][1] * m.m[3][2] -
                  m.m[1][0] * m.m[0][2] * m.m[3][1] -
                  m.m[3][0] * m.m[0][1] * m.m[1][2] +
                  m.m[3][0] * m.m[0][2] * m.m[1][1];

    inv.m[3][3] = m.m[0][0] * m.m[1][1] * m.m[2][2] -
                  m.m[0][0] * m.m[1][2] * m.m[2][1] -
                  m.m[1][0] * m.m[0][1] * m.m[2][2] +
                  m.m[1][0] * m.m[0][2] * m.m[2][1] +
                  m.m[2][0] * m.m[0][1] * m.m[1][2] -
                  m.m[2][0] * m.m[0][2] * m.m[1][1];

    det = m.m[0][0] * inv.m[0][0] + m.m[0][1] * inv.m[1][0] + m.m[0][2] * inv.m[2][0] + m.m[0][3] * inv.m[3][0];

    if (det == 0)
        return mat4_identity(); // Return identity as a fallback

    det = 1.0f / det;

    for (i = 0; i < 4; i++)
        for (int j = 0; j < 4; j++)
            inv.m[i][j] *= det;

    return inv;
}

// Optional: Print matrix for debugging
void mat4_print(Mat4 m)
{
    for (int i = 0; i < 4; i++)
    {
        printf("| ");
        for (int j = 0; j < 4; j++)
        {
            printf("%8.3f ", m.m[i][j]);
        }
        printf("|\n");
    }
}

// Example of rotation around arbitrary axis combined with translation
Mat4 mat4_rotate_translate(float angle_rad, Vec3 axis, Vec3 translation)
{
    Mat4 rotation = mat4_rotate(angle_rad, axis);
    Mat4 translation_matrix = mat4_translate(translation);
    return mat4_multiply(translation_matrix, rotation);
}

Mat4 mat4_create_mvp(
    Vec3 model_position,
    Vec3 model_rotation,
    Vec3 model_scale,
    Vec3 camera_pos,
    Vec3 camera_target,
    Vec3 camera_up,
    float fov,
    float aspect_ratio,
    float near_plane,
    float far_plane)
{
    // 1. Create Model Matrix
    // Translation Matrix
    Mat4 translation = mat4_translate(model_position);

    // Rotation Matrices (assuming rotation_order is X, then Y, then Z)
    Mat4 rotation_x = mat4_rotate_x(model_rotation.x);
    Mat4 rotation_y = mat4_rotate_y(model_rotation.y);
    Mat4 rotation_z = mat4_rotate_z(model_rotation.z);

    // Combined Rotation Matrix
    Mat4 rotation = mat4_multiply(mat4_multiply(rotation_z, rotation_y), rotation_x);

    // Scale Matrix
    Mat4 scale = mat4_scale(model_scale);

    // Combined Model Matrix: Translation * Rotation * Scale
    Mat4 model = mat4_multiply(mat4_multiply(translation, rotation), scale);

    // 2. Create View Matrix
    Mat4 view = mat4_look_at(camera_pos, camera_target, camera_up);

    // 3. Create Projection Matrix
    Mat4 projection = mat4_perspective(fov, aspect_ratio, near_plane, far_plane);

    // 4. Combine to Create MVP Matrix: Projection * View * Model
    Mat4 vp = mat4_multiply(projection, view);
    Mat4 mvp = mat4_multiply(vp, model);

    return mvp;
}

Mat4 mat4_create_model(
    Vec3 model_position,
    Vec3 model_rotation,
    Vec3 model_scale)
{
    // 1. Create Model Matrix
    // Translation Matrix
    Mat4 translation = mat4_translate(model_position);

    // Rotation Matrices (assuming rotation_order is X, then Y, then Z)
    Mat4 rotation_x = mat4_rotate_x(model_rotation.x);
    Mat4 rotation_y = mat4_rotate_y(model_rotation.y);
    Mat4 rotation_z = mat4_rotate_z(model_rotation.z);

    // Combined Rotation Matrix
    Mat4 rotation = mat4_multiply(mat4_multiply(rotation_z, rotation_y), rotation_x);

    // Scale Matrix
    Mat4 scale = mat4_scale(model_scale);

    // Combined Model Matrix: Translation * Rotation * Scale
    Mat4 model = mat4_multiply(mat4_multiply(translation, rotation), scale);

    return model;
}

Mat4 mat4_create_vp(
    Vec3 camera_pos,
    Vec3 camera_target,
    Vec3 camera_up,
    float fov,
    float aspect_ratio,
    float near_plane,
    float far_plane)
{
    // 2. Create View Matrix
    Mat4 view = mat4_look_at(camera_pos, camera_target, camera_up);

    // 3. Create Projection Matrix
    Mat4 projection = mat4_perspective(fov, aspect_ratio, near_plane, far_plane);

    // 4. Combine to Create VP Matrix: Projection * View
    Mat4 vp = mat4_multiply(projection, view);

    return vp;
}

Mat4 mat4_create_mvp_isometric_specific(
    Vec3 model_position,
    Vec3 model_rotation,
    Vec3 model_scale,
    float aspect_ratio,
    float near_plane,
    float far_plane)
{
    // 1. Create Model Matrix
    // Translation Matrix
    Mat4 translation = mat4_translate(model_position);

    // Rotation Matrices for Isometric View
    // Typically, isometric view is achieved by rotating -45 degrees around Y-axis and ~35.264 degrees around X-axis
    float iso_angle_y = degrees_to_radians(45.0f);
    float iso_angle_x = degrees_to_radians(35.264f); // Approximately arctan(1/sqrt(2))

    Mat4 rotation_y = mat4_rotate_y(iso_angle_y);
    Mat4 rotation_x = mat4_rotate_x(iso_angle_x);

    // Combined Rotation Matrix: First Y, then X
    Mat4 rotation = mat4_multiply(rotation_x, rotation_y);

    // Apply additional model rotations
    Mat4 additional_rotation_x = mat4_rotate_x(model_rotation.x);
    Mat4 additional_rotation_y = mat4_rotate_y(model_rotation.y);
    Mat4 additional_rotation_z = mat4_rotate_z(model_rotation.z);
    Mat4 additional_rotation = mat4_multiply(mat4_multiply(additional_rotation_z, additional_rotation_y), additional_rotation_x);

    // Combined Model Matrix: Translation * Isometric Rotation * Additional Rotation * Scale
    Mat4 scale = mat4_scale(model_scale);
    Mat4 model = mat4_multiply(mat4_multiply(mat4_multiply(translation, rotation), additional_rotation), scale);

    // 2. Create Orthographic Projection Matrix
    float ortho_left = -10.0f * aspect_ratio;
    float ortho_right = 10.0f * aspect_ratio;
    float ortho_bottom = -10.0f;
    float ortho_top = 10.0f;

    Mat4 projection = orthographic(ortho_left, ortho_right, ortho_bottom, ortho_top, near_plane, far_plane);

    // 3. Since the isometric rotation is already applied to the model, the view matrix can be identity
    Mat4 view = mat4_identity();

    // 4. Combine to Create MVP Matrix: Projection * View * Model
    Mat4 vp = mat4_multiply(projection, view);
    Mat4 mvp = mat4_multiply(vp, model);

    return mvp;
}
