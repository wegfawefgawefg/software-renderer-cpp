#include "light.h"
#include "sfa.h"
#include "su32a.h"
#include "colors.h"
#include "vec3.h"
#include "utils.h"
#include "globals.h"

Light light_new(Vec3 pos, uint32_t color, float brightness)
{
    Light light = {pos, color, brightness};
    return light;
}

/*
    Calculate the colors of the faces of a mesh based on the lights
    vertices: pre-view world space vertices: (x, y, z, w)
    indices: the indices of the mesh
    colors: the colors of the faces
    lights: the lights
    num_lights: the number of lights

    go through each face, calculate the centroid, distance to light, and brightness
*/
SU32A *lighting_get_face_colors(
    SFA *vertices,
    SU32A *indices,
    SU32A *colors,
    Light *lights,
    int num_lights)
{
    int num_faces = indices->length / 3;
    SU32A *out_colors = su32a_new(num_faces);
    for (int i = 0; i < num_faces; i++)
    {
        // get the indices of the vertices of the face
        int i0 = indices->data[i * 3 + 0];
        int i1 = indices->data[i * 3 + 1];
        int i2 = indices->data[i * 3 + 2];

        // get the vertices of the face
        Vec3 a = vec3_create(
            vertices->data[i0 * 4 + 0],
            vertices->data[i0 * 4 + 1],
            vertices->data[i0 * 4 + 2]);

        Vec3 b = vec3_create(
            vertices->data[i1 * 4 + 0],
            vertices->data[i1 * 4 + 1],
            vertices->data[i1 * 4 + 2]);

        Vec3 c = vec3_create(
            vertices->data[i2 * 4 + 0],
            vertices->data[i2 * 4 + 1],
            vertices->data[i2 * 4 + 2]);

        // calculate the centroid
        Vec3 centroid = vec3_create(0.0f, 0.0f, 0.0f);
        centroid = vec3_add(centroid, a);
        centroid = vec3_add(centroid, b);
        centroid = vec3_add(centroid, c);
        centroid = vec3_div(centroid, 3.0f);

        Vec3 face_normal = vec3_cross(vec3_sub(b, a), vec3_sub(c, a));
        face_normal = vec3_normalize(face_normal);

        // calculate the brightness
        uint32_t light_color_sum = 0;
        const float dist_factor = 1.0f;
        for (int j = 0; j < num_lights; j++)
        {
            Light light = lights[j];
            Vec3 light_dir = vec3_sub(light.pos, centroid);
            float distance = vec3_length(light_dir);
            float dot = vec3_dot(face_normal, vec3_normalize(light_dir));
            dot = fmax(0.0f, dot);

            float magnitude = 1.0f / ((distance * distance) * dist_factor);
            magnitude = fmin(1.0f, magnitude);
            magnitude = fmax(0.0f, magnitude);

            magnitude *= dot;
            magnitude *= light.brightness;

            // clamp again
            magnitude = fmin(1.0f, magnitude);
            magnitude = fmax(0.0f, magnitude);

            // calculate this lights influence
            uint32_t this_light_color = color_fmul(light.color, magnitude);
            light_color_sum = color_add(light_color_sum, this_light_color);
        }
        const float ambient_factor = AMBIENT_LIGHT;
        uint32_t color = colors->data[i];
        uint32_t color_f = color_fmul(color, ambient_factor);
        uint32_t col = color_add(color_f, light_color_sum);
        out_colors->data[i] = col;
    }

    return out_colors;
}

void make_lights(void)
{
    // make an array of 8 lights with malloc
    uint8_t num_lights = 4;
    Light *lights = (Light *)malloc(num_lights * sizeof(Light));

    // put a light at x 0, y 100, z 0
    // lights[0] = light_new(vec3_create(0.0f, 100.0f, 0.0f), COLOR_GREEN, 20.0f);
    // lights[1] = light_new(vec3_create(100.0f, 100.0f, 0.0f), COLOR_RED, 20.0f);

    // make lights, and start them in random positions, spin them around the origin
    for (int i = 0; i < num_lights; i++)
    {
        float time = SDL_GetTicks() / 1000.0f;
        float radius = 300.0f;
        float height_variance = 10.0f;
        float angle = (float)i * (360.0f / (float)num_lights) + time * 30.0f;
        float x = cosf(degrees_to_radians(angle)) * radius;
        float y = sinf(degrees_to_radians(angle)) * height_variance;
        float z = sinf(degrees_to_radians(angle)) * radius;

        // Generate independent R, G, B using hash function with different seeds
        float r = hash(i + 1); // Seed offset for variation
        float g = hash(i + 2);
        float b = hash(i + 3);

        lights[i] = light_new(
            vec3_create(x, y, z),
            color_from_rgb(r * 255, g * 255, b * 255),
            10000.0f);
    }
}