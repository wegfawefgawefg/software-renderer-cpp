#include "projection.h"
#include "sfa.h"

// maps NDC to screen coordinates: (x, y, _z, _w) -> (screen_x, screen_y)
void map_to_screen(const SFA *transformed_sfa, SFA *screen_sfa, int screen_width, int screen_height)
{
    if (!transformed_sfa || !screen_sfa)
        return;

    int vertex_count = transformed_sfa->length / 4;

    for (int i = 0; i < vertex_count; i++)
    {
        float ndc_x = transformed_sfa->data[i * 4 + 0];
        float ndc_y = transformed_sfa->data[i * 4 + 1];

        // Map from NDC to window coordinates
        float screen_x = (ndc_x * 0.5f + 0.5f) * (float)screen_width;
        float screen_y = (1.0f - (ndc_y * 0.5f + 0.5f)) * (float)screen_height; // Y-axis inverted

        // Store as 2D coordinates (x, y)
        screen_sfa->data[i * 2 + 0] = screen_x;
        screen_sfa->data[i * 2 + 1] = screen_y;
    }
}

// maps NDC to screen coordinates: (x, y, _z, _w) -> (screen_x, screen_y, depth)
void map_to_screen_keep_z(const SFA *transformed_sfa, SFA *screen_sfa, int screen_width, int screen_height)
{
    if (!transformed_sfa || !screen_sfa)
        return;

    int vertex_count = transformed_sfa->length / 4;

    for (int i = 0; i < vertex_count; i++)
    {
        float ndc_x = transformed_sfa->data[i * 4 + 0];
        float ndc_y = transformed_sfa->data[i * 4 + 1];
        float ndc_z = transformed_sfa->data[i * 4 + 2];

        // Map from NDC to window coordinates
        float screen_x = (ndc_x * 0.5f + 0.5f) * (float)screen_width;
        float screen_y = (1.0f - (ndc_y * 0.5f + 0.5f)) * (float)screen_height; // Y-axis inverted

        // Store as 2D coordinates with z (x, y, depth)
        screen_sfa->data[i * 3 + 0] = screen_x;
        screen_sfa->data[i * 3 + 1] = screen_y;
        screen_sfa->data[i * 3 + 2] = ndc_z;
    }
}

// perspective divides (already mpv transformed) vertices: (x, y, z, w) -> (x/w, y/w, z/w, 1)
void perspective_divide(SFA *transformed_sfa)
{
    if (!transformed_sfa)
        return;

    int vertex_count = transformed_sfa->length / 4;

    for (int i = 0; i < vertex_count; i++)
    {
        float w = transformed_sfa->data[i * 4 + 3];
        if (w != 0.0f)
        {
            transformed_sfa->data[i * 4 + 0] /= w; // x
            transformed_sfa->data[i * 4 + 1] /= w; // y
            // transformed_sfa->data[i * 4 + 2] /= w;   // z (optional, for depth)
            transformed_sfa->data[i * 4 + 3] = 1.0f; // Set w to 1 after division
        }
        else
        {
            // Handle w == 0 case if necessary
            // For simplicity, we'll leave the coordinates unchanged
        }
    }
}