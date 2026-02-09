#include <stdint.h>

#include "texture.h"
#include "colors.h"
#include "globals.h"
#include "state.h"
#include "vec2.h"
#include "utils.h"
#include "draw_lib.h"
#include "vec3.h"

#define COLOR_PICKER_SIZE 100
#define COLOR_PICKER_MARGIN 10

#define BRIGHTNESS_SLIDER_WIDTH 100
#define BRIGHTNESS_SLIDER_HEIGHT 30
#define BRIGHTNESS_SLIDER_MARGIN 10

void draw_brightness_slider(Texture *pb, State *state)
{
    // Define brightness slider position
    int slider_x = pb->width - BRIGHTNESS_SLIDER_WIDTH - BRIGHTNESS_SLIDER_MARGIN;
    int slider_y = BRIGHTNESS_SLIDER_MARGIN + COLOR_PICKER_SIZE + 10; // 10 pixels below color picker

    for (int x = 0; x < BRIGHTNESS_SLIDER_WIDTH; x++)
    {
        // Calculate brightness (1.0 at left to 0.0 at right)
        float brightness = 1.0f - ((float)x / (float)(BRIGHTNESS_SLIDER_WIDTH - 1));

        // Use current hue and saturation from state
        float hue = state->hue;
        float saturation = state->saturation;

        // Convert HSV to RGB
        uint8_t r, g, b;
        hsv_to_rgb(hue, saturation, brightness, &r, &g, &b);

        // Pack RGB into uint32_t RGBA format (R << 24 | G << 16 | B << 8 | A)
        uint32_t rgb_packed = ((uint32_t)r << 24) | ((uint32_t)g << 16) | ((uint32_t)b << 8) | 0xFF; // A=255

        // Set the pixel color across the slider's height
        for (int y = 0; y < BRIGHTNESS_SLIDER_HEIGHT; y++)
        {
            texture_set(pb, slider_x + x, slider_y + y, rgb_packed);
        }
    }

    // Optional: Draw border around the brightness slider
    uint32_t border_color = 0xFFFFFFFF; // White border
    for (int x = 0; x < BRIGHTNESS_SLIDER_WIDTH; x++)
    {
        texture_set(pb, slider_x + x, slider_y, border_color);
        texture_set(pb, slider_x + x, slider_y + BRIGHTNESS_SLIDER_HEIGHT - 1, border_color);
    }
    for (int y = 0; y < BRIGHTNESS_SLIDER_HEIGHT; y++)
    {
        texture_set(pb, slider_x, slider_y + y, border_color);
        texture_set(pb, slider_x + BRIGHTNESS_SLIDER_WIDTH - 1, slider_y + y, border_color);
    }

    // Draw marker indicating current brightness
    int marker_x = slider_x + (int)((1.0f - state->brightness) * (BRIGHTNESS_SLIDER_WIDTH - 1));
    int marker_y = slider_y + BRIGHTNESS_SLIDER_HEIGHT / 2;
    uint32_t marker_color = 0xFF000000; // Black marker

    // Draw a small vertical line as marker
    for (int dy = -2; dy <= 2; dy++)
    {
        int px = marker_x;
        int py = marker_y + dy;
        if (px >= slider_x && px < slider_x + BRIGHTNESS_SLIDER_WIDTH &&
            py >= slider_y && py < slider_y + BRIGHTNESS_SLIDER_HEIGHT)
        {
            texture_set(pb, px, py, marker_color);
        }
    }
}

void draw_selected_color(Texture *pb, uint32_t color)
{
    int color_display_size = 30;
    int display_x = COLOR_PICKER_MARGIN;
    int display_y = COLOR_PICKER_MARGIN;

    for (int x = 0; x < color_display_size; x++)
    {
        for (int y = 0; y < color_display_size; y++)
        {
            texture_set(pb, display_x + x, display_y + y, color);
        }
    }

    // Draw border around the color display
    uint32_t border_color = 0xFFFFFFFF; // White border
    for (int x = 0; x < color_display_size; x++)
    {
        texture_set(pb, display_x + x, display_y, border_color);
        texture_set(pb, display_x + x, display_y + color_display_size - 1, border_color);
    }
    for (int y = 0; y < color_display_size; y++)
    {
        texture_set(pb, display_x, display_y + y, border_color);
        texture_set(pb, display_x + color_display_size - 1, display_y + y, border_color);
    }
}

// Utility function to clamp values
float clamp(float value, float min, float max)
{
    if (value < min)
        return min;
    if (value > max)
        return max;
    return value;
}
// Function to clamp a value between min and max
static float clamp_float(float value, float min, float max)
{
    if (value < min)
        return min;
    if (value > max)
        return max;
    return value;
}
// Function to handle color picker and brightness slider input
void handle_color_picker_input(State *state, Texture *pb, Vec2 mouse_pos, uint32_t mouse_buttons)
{
    // Define hue-saturation picker position
    int picker_x = pb->width - COLOR_PICKER_SIZE - COLOR_PICKER_MARGIN;
    int picker_y = COLOR_PICKER_MARGIN;

    // Define brightness slider position
    int slider_x = pb->width - BRIGHTNESS_SLIDER_WIDTH - BRIGHTNESS_SLIDER_MARGIN;
    int slider_y = BRIGHTNESS_SLIDER_MARGIN + COLOR_PICKER_SIZE + 10; // 10 pixels below color picker

    // Check if mouse is within the hue-saturation picker
    bool within_picker = (mouse_pos.x >= picker_x) &&
                         (mouse_pos.x < picker_x + COLOR_PICKER_SIZE) &&
                         (mouse_pos.y >= picker_y) &&
                         (mouse_pos.y < picker_y + COLOR_PICKER_SIZE);

    // Check if mouse is within the brightness slider
    bool within_slider = (mouse_pos.x >= slider_x) &&
                         (mouse_pos.x < slider_x + BRIGHTNESS_SLIDER_WIDTH) &&
                         (mouse_pos.y >= slider_y) &&
                         (mouse_pos.y < slider_y + BRIGHTNESS_SLIDER_HEIGHT);

    // Handle hue-saturation picker input
    if (within_picker && (mouse_buttons & SDL_BUTTON(SDL_BUTTON_LEFT)))
    {
        // Calculate hue based on x position
        float relative_x = clamp_float((float)(mouse_pos.x - picker_x), 0.0f, (float)(COLOR_PICKER_SIZE - 1));
        float hue = (relative_x / (float)(COLOR_PICKER_SIZE - 1)) * 360.0f;

        // Calculate saturation based on y position (inverted, as y increases downward)
        float relative_y = clamp_float((float)(mouse_pos.y - picker_y), 0.0f, (float)(COLOR_PICKER_SIZE - 1));
        float saturation = 1.0f - (relative_y / (float)(COLOR_PICKER_SIZE - 1));

        // Update state hue and saturation
        state->hue = hue;
        state->saturation = saturation;

        // Update state->color based on new hue and saturation, keeping brightness
        uint8_t r, g, b;
        hsv_to_rgb(state->hue, state->saturation, state->brightness, &r, &g, &b);
        state->color = ((uint32_t)r << 24) | ((uint32_t)g << 16) | ((uint32_t)b << 8) | 0xFF; // RGBA

        // Optional: Print the updated HSV and RGB values
        printf("Selected Color: 0x%08X (H: %.2f°, S: %.2f, V: %.2f)\n", state->color, state->hue, state->saturation, state->brightness);
    }

    // Handle brightness slider input
    if (within_slider && (mouse_buttons & SDL_BUTTON(SDL_BUTTON_LEFT)))
    {
        // Calculate brightness based on x position
        float relative_x = clamp_float((float)(mouse_pos.x - slider_x), 0.0f, (float)(BRIGHTNESS_SLIDER_WIDTH - 1));
        float brightness = 1.0f - (relative_x / (float)(BRIGHTNESS_SLIDER_WIDTH - 1));

        // Update state brightness
        state->brightness = brightness;

        // Update state->color based on current hue, saturation, and new brightness
        uint8_t r, g, b;
        hsv_to_rgb(state->hue, state->saturation, state->brightness, &r, &g, &b);
        state->color = ((uint32_t)r << 24) | ((uint32_t)g << 16) | ((uint32_t)b << 8) | 0xFF; // RGBA

        // Optional: Print the updated HSV and RGB values
        printf("Selected Color: 0x%08X (H: %.2f°, S: %.2f, V: %.2f)\n", state->color, state->hue, state->saturation, state->brightness);
    }
}

// Function to draw the 2D hue-saturation color picker
void draw_hue_saturation_picker(Texture *pb, State *state)
{
    // Define color picker position
    int picker_x = pb->width - COLOR_PICKER_SIZE - COLOR_PICKER_MARGIN;
    int picker_y = COLOR_PICKER_MARGIN;

    for (int x = 0; x < COLOR_PICKER_SIZE; x++)
    {
        // Calculate hue (0 to 360 degrees)
        float hue = ((float)x / (float)(COLOR_PICKER_SIZE - 1)) * 360.0f;

        for (int y = 0; y < COLOR_PICKER_SIZE; y++)
        {
            // Calculate saturation (1.0 at top to 0.0 at bottom)
            float saturation = 1.0f - ((float)y / (float)(COLOR_PICKER_SIZE - 1));

            // Use current brightness from state
            float brightness = state->brightness;

            // Convert HSV to RGB
            uint8_t r, g, b;
            hsv_to_rgb(hue, saturation, brightness, &r, &g, &b);

            // Pack RGB into uint32_t RGBA format (R << 24 | G << 16 | B << 8 | A)
            uint32_t rgb_packed = ((uint32_t)r << 24) | ((uint32_t)g << 16) | ((uint32_t)b << 8) | 0xFF; // A=255

            // Set the pixel color
            texture_set(pb, picker_x + x, picker_y + y, rgb_packed);
        }
    }

    // Optional: Draw border around the color picker
    uint32_t border_color = 0xFFFFFFFF; // White border
    for (int x = 0; x < COLOR_PICKER_SIZE; x++)
    {
        texture_set(pb, picker_x + x, picker_y, border_color);
        texture_set(pb, picker_x + x, picker_y + COLOR_PICKER_SIZE - 1, border_color);
    }
    for (int y = 0; y < COLOR_PICKER_SIZE; y++)
    {
        texture_set(pb, picker_x, picker_y + y, border_color);
        texture_set(pb, picker_x + COLOR_PICKER_SIZE - 1, picker_y + y, border_color);
    }

    // Draw marker indicating current hue and saturation
    int marker_x = picker_x + (int)((state->hue / 360.0f) * (COLOR_PICKER_SIZE - 1));
    int marker_y = picker_y + (int)((1.0f - state->saturation) * (COLOR_PICKER_SIZE - 1));
    uint32_t marker_color = 0xFF000000; // Black marker

    // Draw a small crosshair
    for (int dx = -2; dx <= 2; dx++)
    {
        for (int dy = -2; dy <= 2; dy++)
        {
            int px = marker_x + dx;
            int py = marker_y + dy;
            if (px >= picker_x && px < picker_x + COLOR_PICKER_SIZE &&
                py >= picker_y && py < picker_y + COLOR_PICKER_SIZE)
            {
                texture_set(pb, px, py, marker_color);
            }
        }
    }
}

// {
//     // Draw the hue-saturation color picker
//     draw_hue_saturation_picker(pb, state);
//     draw_brightness_slider(pb, state);
//     draw_selected_color(pb, state->color);

//     // Handle color picker input
//     MouseState mouseState;
//     mouseState.buttons = get_mouse_buttons();
//     mouseState.pos = get_mouse_pos();
//     handle_color_picker_input(state, pb, ivec2_to_vec2(mouseState.pos), mouseState.buttons);
// }

// // draw a rect at the cursor of given size
// const int cursor_size = 5;
// draw_rect(pb, mouse_pos.x - cursor_size / 2, mouse_pos.y - cursor_size / 2, cursor_size, cursor_size, color_set_alpha(state->color, 255));

// // if mouse is pressed, get the coord from the face buffer, set the color to red in the jet model colors list
// {
//     MouseState mouseState;
//     mouseState.buttons = get_mouse_buttons();
//     mouseState.pos = get_mouse_pos();

//     // picker
//     int picker_x = pb->width - COLOR_PICKER_SIZE - COLOR_PICKER_MARGIN;
//     int picker_y = COLOR_PICKER_MARGIN;
//     if (is_left_mouse_button_down(&mouseState))
//     {
//         for (int y = mouseState.pos.y - cursor_size / 2; y < mouseState.pos.y + cursor_size / 2; y++)
//         {
//             for (int x = mouseState.pos.x - cursor_size / 2; x < mouseState.pos.x + cursor_size / 2; x++)
//             {
//                 // check if in picker
//                 // Check if mouse is within the color picker area
//                 bool within_picker = (x >= picker_x) &&
//                                      (x < picker_x + COLOR_PICKER_SIZE) &&
//                                      (y >= picker_y) &&
//                                      (y < picker_y + COLOR_PICKER_SIZE);
//                 if (!within_picker && x >= 0 && x < pb->width && y >= 0 && y < pb->height)
//                 {

//                     // get the face under the cursor
//                     uint32_t face = texture_get(face_buffer, x, y);
//                     // skip if value is uint32_max
//                     if (face != UINT32_MAX)
//                     {
//                         mesh->colors->data[face] = state->color;
//                     }
//                 }
//             }
//         }
//     }
// }