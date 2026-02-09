#include "sfa.h"

#include <stdlib.h>
#include <math.h>
#include "vec3.h"
#include "vec4.h"
#include "mat4.h"

SFA *sfa_new(int length)
{
    SFA *sfa = (SFA *)malloc(sizeof(SFA));
    sfa->length = length;
    sfa->data = (float *)malloc(sizeof(float) * length);
    return sfa;
}

void sfa_free(SFA *sfa)
{
    free(sfa->data);
    free(sfa);
}

//////////////////////// TRANSFORM FUNCTIONS ////////////////////////

void sfa_rotate(SFA *sfa, Vec3 rotation)
{
    for (int i = 0; i < sfa->length; i += 3)
    {
        float x = sfa->data[i];
        float y = sfa->data[i + 1];
        float z = sfa->data[i + 2];

        // Rotate around X-axis
        float temp_y = y;
        float temp_z = z;
        y = temp_y * cosf(rotation.x) - temp_z * sinf(rotation.x);
        z = temp_y * sinf(rotation.x) + temp_z * cosf(rotation.x);

        // Rotate around Y-axis
        float temp_x = x;
        temp_z = z;
        x = temp_x * cosf(rotation.y) + temp_z * sinf(rotation.y);
        z = -temp_x * sinf(rotation.y) + temp_z * cosf(rotation.y);

        // Rotate around Z-axis
        temp_x = x;
        temp_y = y;
        x = temp_x * cosf(rotation.z) - temp_y * sinf(rotation.z);
        y = temp_x * sinf(rotation.z) + temp_y * cosf(rotation.z);

        sfa->data[i] = x;
        sfa->data[i + 1] = y;
        sfa->data[i + 2] = z;
    }
}

void sfa_scale(SFA *sfa, Vec3 scale)
{
    for (int i = 0; i < sfa->length; i += 3)
    {
        sfa->data[i] *= scale.x;
        sfa->data[i + 1] *= scale.y;
        sfa->data[i + 2] *= scale.z;
    }
}

void sfa_translate(SFA *sfa, Vec3 translation)
{
    for (int i = 0; i < sfa->length; i += 3)
    {
        sfa->data[i] += translation.x;
        sfa->data[i + 1] += translation.y;
        sfa->data[i + 2] += translation.z;
    }
}

// Orthographic projection onto the XY-plane
SFA *sfa_orthographic_projection_xy(const SFA *sfa)
{
    // new sfa will only be xyxy, but passed in one was xyzxyz
    SFA *projected = sfa_new(sfa->length / 3 * 2);
    if (!projected)
    {
        return NULL;
    }

    for (int i = 0, j = 0; i < sfa->length; i += 3, j += 2)
    {
        projected->data[j] = sfa->data[i];
        projected->data[j + 1] = sfa->data[i + 1];
    }

    return projected;
}

// Define the isometric projection matrix components
#define ISO_ANGLE_X (35.264f * (M_PI / 180.0f)) // Convert degrees to radians
#define ISO_ANGLE_Y (45.0f * (M_PI / 180.0f))

SFA *sfa_isometric_projection(const SFA *sfa)
{
    if (sfa->length % 3 != 0)
    {
        // Invalid input length; must be a multiple of 3
        return NULL;
    }

    int num_vertices = sfa->length / 3;
    // Each vertex will be projected to 2D, so total length is num_vertices * 2
    SFA *projected = sfa_new(num_vertices * 2);
    if (!projected)
    {
        return NULL;
    }

    // Precompute sine and cosine of rotation angles
    float cos_x = cosf(ISO_ANGLE_X);
    float sin_x = sinf(ISO_ANGLE_X);
    float cos_y = cosf(ISO_ANGLE_Y);
    float sin_y = sinf(ISO_ANGLE_Y);

    // Combined projection matrix elements
    float m00 = cos_y;
    float m01 = 0;
    float m02 = -sin_y;

    float m10 = sin_x * sin_y;
    float m11 = cos_x;
    float m12 = sin_x * cos_y;

    for (int i = 0, j = 0; i < sfa->length; i += 3, j += 2)
    {
        // Original coordinates
        float x = sfa->data[i];
        float y = sfa->data[i + 1];
        float z = sfa->data[i + 2];

        // Apply the combined isometric projection matrix
        float x_proj = x * m00 + y * m01 + z * m02;
        float y_proj = x * m10 + y * m11 + z * m12;

        projected->data[j] = x_proj;
        projected->data[j + 1] = y_proj;
    }

    return projected;
}

// Applies mvp matrix transformation to input_sfa: (x,y,z) => (x,y,z,1) => (x',y',z',w')
SFA *sfa_transform_vertices(const SFA *input_sfa, const Mat4 *mvp)
{
    if (!input_sfa || !mvp)
        return NULL;

    int vertex_count = input_sfa->length / 3;
    SFA *transformed_sfa = sfa_new(vertex_count * 4); // Store as vec4 (homogeneous coordinates)

    if (!transformed_sfa)
    {
        // Handle memory allocation failure
        return NULL;
    }

    for (int i = 0; i < vertex_count; i++)
    {
        // Original vertex
        Vec3 original = {
            input_sfa->data[i * 3 + 0],
            input_sfa->data[i * 3 + 1],
            input_sfa->data[i * 3 + 2]};

        // Convert to Vec4 (homogeneous coordinates)
        Vec4 vertex = {original.x, original.y, original.z, 1.0f};

        // Apply MVP transformation
        Vec4 transformed = mat4_multiply_vec4(*mvp, vertex);

        // Store transformed vertex
        transformed_sfa->data[i * 4 + 0] = transformed.x;
        transformed_sfa->data[i * 4 + 1] = transformed.y;
        transformed_sfa->data[i * 4 + 2] = transformed.z;
        transformed_sfa->data[i * 4 + 3] = transformed.w;
    }

    return transformed_sfa;
}

// Function to create an arrow along the X-axis
SFA *create_x_axis_arrow(void)
{
    // Two points: origin and (1, 0, 0)
    SFA *arrow_sfa = sfa_new(6); // 2 vertices * 3 components each

    if (!arrow_sfa)
    {
        // Handle memory allocation failure
        return NULL;
    }

    // Vertex 1: Origin
    arrow_sfa->data[0] = 0.0f; // x0
    arrow_sfa->data[1] = 0.0f; // y0
    arrow_sfa->data[2] = 0.0f; // z0

    // Vertex 2: Point along X-axis
    arrow_sfa->data[3] = 1.0f; // x1
    arrow_sfa->data[4] = 0.0f; // y1
    arrow_sfa->data[5] = 0.0f; // z1

    return arrow_sfa;
}
