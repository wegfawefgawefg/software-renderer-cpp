#include "draw.h"
#include "primitives.h"
#include "utils.h"
#include "vec2.h"
#include "f_texture.h"

void draw_line(Texture *pb, int x0, int y0, int x1, int y1, uint32_t color)
{
    // Bresenham's line algorithm
    int dx = abs(x1 - x0);
    int dy = abs(y1 - y0);

    int sx = x0 < x1 ? 1 : -1;
    int sy = y0 < y1 ? 1 : -1;

    int err = dx - dy;
    int e2;

    while (1)
    {
        texture_set_alpha(pb, x0, y0, color);

        if (x0 == x1 && y0 == y1)
        {
            break;
        }

        e2 = 2 * err;
        if (e2 > -dy)
        {
            err -= dy;
            x0 += sx;
        }
        if (e2 < dx)
        {
            err += dx;
            y0 += sy;
        }
    }
}

void draw_lines(Texture *pb, SFA *points, uint32_t color)
{
    for (int i = 0; i < points->length; i += 2)
    {
        int x0 = (int)points->data[i];
        int y0 = (int)points->data[i + 1];
        int x1 = (int)points->data[i + 2];
        int y1 = (int)points->data[i + 3];
        draw_line(pb, x0, y0, x1, y1, color);
    }
}

void draw_rect(Texture *pb, int x, int y, int w, int h, uint32_t color)
{
    for (int i = x; i < x + w; i++)
    {
        for (int j = y; j < y + h; j++)
        {
            texture_set_alpha(pb, i, j, color);
        }
    }
}

void draw_rect_lines(Texture *pb, int x, int y, int w, int h, uint32_t color)
{
    draw_line(pb, x, y, x + w, y, color);
    draw_line(pb, x + w, y, x + w, y + h, color);
    draw_line(pb, x + w, y + h, x, y + h, color);
    draw_line(pb, x, y + h, x, y, color);
}

void draw_triangle_lines(Texture *pb, Triangle t, uint32_t color)
{
    draw_line(pb, t.p1.x, t.p1.y, t.p2.x, t.p2.y, color);
    draw_line(pb, t.p2.x, t.p2.y, t.p3.x, t.p3.y, color);
    draw_line(pb, t.p3.x, t.p3.y, t.p1.x, t.p1.y, color);
}

static inline int edge_function(IVec2 a, IVec2 b, IVec2 p)
{
    return (b.x - a.x) * (p.y - a.y) - (b.y - a.y) * (p.x - a.x);
}

void draw_triangle(Texture *pb, Triangle t, uint32_t color)
{
    IVec2 p1 = vec2_to_ivec2(t.p1);
    IVec2 p2 = vec2_to_ivec2(t.p2);
    IVec2 p3 = vec2_to_ivec2(t.p3);

    // Compute bounding box
    int minX = imin(imin(p1.x, p2.x), p3.x);
    int minY = imin(imin(p1.y, p2.y), p3.y);
    int maxX = imax(imax(p1.x, p2.x), p3.x);
    int maxY = imax(imax(p1.y, p2.y), p3.y);

    // Clip against screen bounds
    minX = imax(minX, 0);
    minY = imax(minY, 0);
    maxX = imin(maxX, pb->width - 1);
    maxY = imin(maxY, pb->height - 1);

    // Determine triangle orientation
    int area = edge_function(p1, p2, p3);
    bool clockwise = area <= 0;

    IVec2 p;
    for (p.y = minY; p.y <= maxY; p.y++)
    {
        for (p.x = minX; p.x <= maxX; p.x++)
        {
            int w1 = edge_function(p1, p2, p);
            int w2 = edge_function(p2, p3, p);
            int w3 = edge_function(p3, p1, p);

            // If p is on or inside all edges, render pixel
            if (clockwise)
            {
                if (w1 <= 0 && w2 <= 0 && w3 <= 0)
                {
                    texture_set_alpha(pb, p.x, p.y, color);
                }
            }
        }
    }
}

// draw_triangle_centroid_z_per_pixel_z_check(pb, z_buffer, t, color, z);
/*
    when drawing a pixel we will sample the z buffer at that position
    if the z value of the pixel is less than the z value in the z buffer we will draw the pixel and update the z buffer
    else we will skip the pixel

    can use f_texture get and set
*/
void draw_triangle_centroid_z_per_pixel_z_check(Texture *pb, FTexture *z_buffer, Triangle t, uint32_t color, float z)
{
    IVec2 p1 = vec2_to_ivec2(t.p1);
    IVec2 p2 = vec2_to_ivec2(t.p2);
    IVec2 p3 = vec2_to_ivec2(t.p3);

    // Compute bounding box
    int minX = imin(imin(p1.x, p2.x), p3.x);
    int minY = imin(imin(p1.y, p2.y), p3.y);
    int maxX = imax(imax(p1.x, p2.x), p3.x);
    int maxY = imax(imax(p1.y, p2.y), p3.y);

    // Clip against screen bounds
    minX = imax(minX, 0);
    minY = imax(minY, 0);
    maxX = imin(maxX, pb->width - 1);
    maxY = imin(maxY, pb->height - 1);

    // Determine triangle orientation
    int area = edge_function(p1, p2, p3);
    bool clockwise = area <= 0;

    IVec2 p;
    for (p.y = minY; p.y <= maxY; p.y++)
    {
        for (p.x = minX; p.x <= maxX; p.x++)
        {
            int w1 = edge_function(p1, p2, p);
            int w2 = edge_function(p2, p3, p);
            int w3 = edge_function(p3, p1, p);

            // If p is on or inside all edges, render pixel
            if (clockwise)
            {
                if (w1 <= 0 && w2 <= 0 && w3 <= 0)
                {
                    // get z value from z buffer
                    float z_buffer_value = f_texture_get(z_buffer, p.x, p.y);
                    if (z > 80.0f && z < z_buffer_value)
                    {
                        texture_set_alpha(pb, p.x, p.y, color);
                        f_texture_set(z_buffer, p.x, p.y, z);
                    }
                }
            }
        }
    }
}

// Swap function for vertices
void swap_vec2(Vec2 *a, Vec2 *b)
{
    Vec2 temp = *a;
    *a = *b;
    *b = temp;
}

// Sort the triangle vertices by Y-coordinate ascending (v0.y <= v1.y <= v2.y)
void sort_vertices_by_y(Vec2 *v0, Vec2 *v1, Vec2 *v2)
{
    if (v1->y < v0->y)
        swap_vec2(v0, v1);
    if (v2->y < v0->y)
        swap_vec2(v0, v2);
    if (v2->y < v1->y)
        swap_vec2(v1, v2);
}

void sort_verticies_by_y_and_swap_uv_indices(Vec2 *v0, Vec2 *v1, Vec2 *v2, Vec2 *uv0, Vec2 *uv1, Vec2 *uv2)
{
    if (v1->y < v0->y)
    {
        swap_vec2(v0, v1);
        swap_vec2(uv0, uv1);
    }
    if (v2->y < v0->y)
    {
        swap_vec2(v0, v2);
        swap_vec2(uv0, uv2);
    }
    if (v2->y < v1->y)
    {
        swap_vec2(v1, v2);
        swap_vec2(uv1, uv2);
    }
}

void draw_triangle_scanline_constant_z(Texture *pb, FTexture *z_buffer, Triangle t, uint32_t color, float z)
{
    Vec2 v0 = t.p1;
    Vec2 v1 = t.p2;
    Vec2 v2 = t.p3;

    // Sort vertices by Y-coordinate ascending
    sort_vertices_by_y(&v0, &v1, &v2);

    // Compute inverse slopes
    // float inv_slope_1 = 0, inv_slope_2 = 0;

    // Calculate the height of the triangle
    float total_height = v2.y - v0.y;
    if (total_height == 0)
        return; // Degenerate triangle

    // Rasterize the triangle
    for (int y = (int)ceilf(v0.y); y <= (int)floorf(v2.y); y++)
    {
        bool second_half = y > v1.y || v1.y == v0.y;
        float segment_height = second_half ? v2.y - v1.y : v1.y - v0.y;
        if (segment_height == 0)
            continue; // Avoid division by zero

        float alpha = (y - v0.y) / total_height;
        float beta = (y - (second_half ? v1.y : v0.y)) / segment_height;

        Vec2 A = {v0.x + (v2.x - v0.x) * alpha, (float)y};
        Vec2 B = second_half ? (Vec2){v1.x + (v2.x - v1.x) * beta, (float)y}
                             : (Vec2){v0.x + (v1.x - v0.x) * beta, (float)y};

        // Ensure A.x <= B.x
        if (A.x > B.x)
        {
            Vec2 temp = A;
            A = B;
            B = temp;
        }

        // Clip X coordinates to screen bounds
        int x_start = (int)ceilf(fmaxf(A.x, 0));
        int x_end = (int)floorf(fminf(B.x, pb->width - 1));

        if (y < 0 || y >= pb->height)
            continue;

        for (int x = x_start; x <= x_end; x++)
        {
            // Z-buffer check and update
            float z_buffer_value = f_texture_get(z_buffer, x, y);
            if (z < z_buffer_value)
            {
                texture_set_alpha(pb, x, y, color);
                f_texture_set(z_buffer, x, y, z);
            }
        }
    }
}

void draw_triangle_scanline_constant_z_with_face_buffer(
    Texture *pb,
    FTexture *z_buffer,
    Triangle t,
    uint32_t color,
    float z,
    Texture *face_buffer,
    uint32_t face)
{
    Vec2 v0 = t.p1;
    Vec2 v1 = t.p2;
    Vec2 v2 = t.p3;

    // Sort vertices by Y-coordinate ascending
    sort_vertices_by_y(&v0, &v1, &v2);

    // Compute inverse slopes
    // float inv_slope_1 = 0, inv_slope_2 = 0;

    // Calculate the height of the triangle
    float total_height = v2.y - v0.y;
    if (total_height == 0)
        return; // Degenerate triangle

    // Rasterize the triangle
    for (int y = (int)ceilf(v0.y); y <= (int)floorf(v2.y); y++)
    {
        bool second_half = y > v1.y || v1.y == v0.y;
        float segment_height = second_half ? v2.y - v1.y : v1.y - v0.y;
        if (segment_height == 0)
            continue; // Avoid division by zero

        float alpha = (y - v0.y) / total_height;
        float beta = (y - (second_half ? v1.y : v0.y)) / segment_height;

        Vec2 A = {v0.x + (v2.x - v0.x) * alpha, (float)y};
        Vec2 B = second_half ? (Vec2){v1.x + (v2.x - v1.x) * beta, (float)y}
                             : (Vec2){v0.x + (v1.x - v0.x) * beta, (float)y};

        // Ensure A.x <= B.x
        if (A.x > B.x)
        {
            Vec2 temp = A;
            A = B;
            B = temp;
        }

        // Clip X coordinates to screen bounds
        int x_start = (int)ceilf(fmaxf(A.x, 0));
        int x_end = (int)floorf(fminf(B.x, pb->width - 1));

        if (y < 0 || y >= pb->height)
            continue;

        for (int x = x_start; x <= x_end; x++)
        {
            // Z-buffer check and update
            float z_buffer_value = f_texture_get(z_buffer, x, y);
            if (z < z_buffer_value)
            {
                texture_set_alpha(pb, x, y, color);
                f_texture_set(z_buffer, x, y, z);
                texture_set(face_buffer, x, y, face);
            }
        }
    }
}

// Function to draw a triangle with scanline rasterization and texture sampling
void draw_triangle_scanline_with_texture(
    Texture *pb,
    Texture *texture,
    FTexture *z_buffer,
    Triangle t,
    Triangle t_uv,
    float z)
{
    // if texture is null, return
    if (!texture)
        return;

    // Extract vertex positions
    Vec2 v0 = t.p1;
    Vec2 v1 = t.p2;
    Vec2 v2 = t.p3;

    // Extract UV coordinates corresponding to each vertex
    Vec2 uv0 = t_uv.p1;
    Vec2 uv1 = t_uv.p2;
    Vec2 uv2 = t_uv.p3;

    // Sort vertices by Y-coordinate ascending
    sort_verticies_by_y_and_swap_uv_indices(&v0, &v1, &v2, &uv0, &uv1, &uv2);

    // Calculate the total height of the triangle
    float total_height = v2.y - v0.y;
    if (total_height == 0.0f)
        return; // Degenerate triangle

    // Rasterize the triangle using scanline approach
    for (int y = (int)ceilf(v0.y); y <= (int)floorf(v2.y); y++)
    {
        bool second_half = y > v1.y || v1.y == v0.y;
        float segment_height = second_half ? (v2.y - v1.y) : (v1.y - v0.y);
        if (segment_height == 0.0f)
            continue; // Avoid division by zero

        float alpha = (float)(y - v0.y) / total_height;
        float beta = second_half ? (float)(y - v1.y) / (v2.y - v1.y) : (float)(y - v0.y) / (v1.y - v0.y);

        // Compute intersection points on the left and right edges
        Vec2 A = {v0.x + (v2.x - v0.x) * alpha, (float)y};
        Vec2 B = second_half ? (Vec2){v1.x + (v2.x - v1.x) * beta, (float)y} : (Vec2){v0.x + (v1.x - v0.x) * beta, (float)y};

        // Compute corresponding UV coordinates at A and B
        Vec2 A_uv = {uv0.x + (uv2.x - uv0.x) * alpha, uv0.y + (uv2.y - uv0.y) * alpha};
        Vec2 B_uv = second_half ? (Vec2){uv1.x + (uv2.x - uv1.x) * beta, uv1.y + (uv2.y - uv1.y) * beta} : (Vec2){uv0.x + (uv1.x - uv0.x) * beta, uv0.y + (uv1.y - uv0.y) * beta};

        // Ensure A.x <= B.x for proper scanline processing
        if (A.x > B.x)
        {
            Vec2 temp = A;
            A = B;
            B = temp;

            Vec2 temp_uv = A_uv;
            A_uv = B_uv;
            B_uv = temp_uv;
        }

        // Clip X coordinates to screen bounds
        int x_start = (int)ceilf(fmaxf(A.x, 0.0f));
        int x_end = (int)floorf(fminf(B.x, (float)(pb->width - 1)));

        if (y < 0 || y >= pb->height)
            continue; // Skip scanlines outside the screen

        // Calculate the horizontal distance between A and B
        float scanline_length = B.x - A.x;
        if (scanline_length == 0.0f)
            continue; // Avoid division by zero

        // Calculate the step increments for U and V per pixel
        float u_step = (B_uv.x - A_uv.x) / scanline_length;
        float v_step = (B_uv.y - A_uv.y) / scanline_length;

        // Starting UV coordinates for the scanline
        float u = A_uv.x + (x_start - A.x) * u_step;
        float v = A_uv.y + (x_start - A.x) * v_step;

        for (int x = x_start; x <= x_end; x++)
        {
            // Clamp UV coordinates to [0, 1] to avoid sampling outside the texture
            // modulus the u and v values to keep them in the range [0, 1]
            // the values right now can even be negative so we have to manually do this
            if (u > 1.0f)
            {
                // lets say u is 14.2
                // 14.2 % 1 = 0.2
                u = u - (int)u;
            }
            else if (u < 0.0f)
            {
                // lets say u is -14.2
                // -14.2 % 1 = 0.8
                u = 1.0f + u - (int)u;
            }

            if (v > 1.0f)
            {
                v = v - (int)v;
            }
            else if (v < 0.0f)
            {
                v = 1.0f + v - (int)v;
            }

            float clamped_u = fminf(fmaxf(u, 0.0f), 1.0f);
            float clamped_v = fminf(fmaxf(v, 0.0f), 1.0f);
            // // invert u and v
            clamped_u = 1.0f - clamped_u;
            clamped_v = 1.0f - clamped_v;

            // Convert UV coordinates to texture space indices
            // print the clamped u and v
            // printf("clamped u: %f, clamped v: %f\n", clamped_u, clamped_v);
            // print the texture dimensions
            // printf("texture width: %d, texture height: %d\n", texture->width, texture->height);
            int tex_x = (int)(clamped_u * (texture->width - 1));
            int tex_y = (int)(clamped_v * (texture->height - 1));

            // Sample the texture color at (tex_x, tex_y)
            uint32_t sampled_color = texture_get(texture, tex_x, tex_y);

            // Z-buffer check and update
            float z_buffer_value = f_texture_get(z_buffer, x, y);
            if (z < z_buffer_value)
            {
                // Update the pixel color in the framebuffer
                texture_set_alpha(pb, x, y, sampled_color);

                // Update the Z-buffer with the new Z value
                f_texture_set(z_buffer, x, y, z);

                // Optionally, set the face identifier in the face buffer
                // if (face_buffer != NULL)
                //     texture_set(face_buffer, x, y, face);
            }

            // Increment UV coordinates for the next pixel
            u += u_step;
            v += v_step;
        }
    }
}

// //////////////////////// COMPOUND DRAWING FUNCTIONS ////////////////////////
void draw_cursor(Texture *pb, int x, int y, int size, uint32_t color)
{
    draw_line(pb, x - size, y, x + size, y, color);
    draw_line(pb, x, y - size, x, y + size, color);
}

void draw_arrow(Texture *pb, int x, int y, int size, uint32_t color)
{
    draw_line(pb, x, y, x + size, y, color);
    draw_line(pb, x + size, y, x + size - size / 4, y - size / 4, color);
    draw_line(pb, x + size, y, x + size - size / 4, y + size / 4, color);
}

void draw_grid(Texture *pb, IVec2 start, IVec2 end, int spacing, uint32_t color)
{
    for (int x = start.x; x < end.x; x += spacing)
    {
        draw_line(pb, x, start.y, x, end.y, color);
    }
    for (int y = start.y; y < end.y; y += spacing)
    {
        draw_line(pb, start.x, y, end.x, y, color);
    }

    // draw the rightmost and bottommost lines
    draw_line(pb, end.x, start.y, end.x, end.y, color);
    draw_line(pb, start.x, end.y, end.x, end.y, color);
}

// draw a grid with dots instead of , puts a dot at each intersection
void draw_grid_dots(Texture *pb, IVec2 start, IVec2 end, int spacing, uint32_t color)
{
    for (int x = start.x; x < end.x + 1; x += spacing)
    {
        for (int y = start.y; y < end.y + 1; y += spacing)
        {
            texture_set(pb, x, y, color);
        }
    }

    // draw the rightmost and bottommost line of dots
    for (int x = start.x; x < end.x; x += spacing)
    {
        texture_set(pb, x, end.y, color);
    }

    for (int y = start.y; y < end.y; y += spacing)
    {
        texture_set(pb, end.x, y, color);
    }

    // bottom right corner
    texture_set(pb, end.x, end.y, color);
}

// draw_checkerboard
// takes in two colors and the size of the squares
//    draw_checkerboard(pb, ivec2_create(0, 0), ivec2_create(pb->width - 1, pb->height - 1), 20, 0xFF333333, 0xFF444444);

void draw_checkerboard(Texture *pb, IVec2 start, IVec2 end, int spacing, uint32_t color1, uint32_t color2)
{
    for (int x = start.x; x < end.x + 1; x += spacing)
    {
        for (int y = start.y; y < end.y + 1; y += spacing)
        {
            if ((x / spacing + y / spacing) % 2 == 0)
            {
                draw_rect(pb, x, y, spacing, spacing, color1);
            }
            else
            {
                draw_rect(pb, x, y, spacing, spacing, color2);
            }
        }
    }
}

void draw_ortho_quad_lines(Texture *pb, Quad *quad, uint32_t color)
{
    draw_line(pb, quad->p1.x, quad->p1.y, quad->p2.x, quad->p2.y, color);
    draw_line(pb, quad->p2.x, quad->p2.y, quad->p3.x, quad->p3.y, color);
    draw_line(pb, quad->p3.x, quad->p3.y, quad->p4.x, quad->p4.y, color);
    draw_line(pb, quad->p4.x, quad->p4.y, quad->p1.x, quad->p1.y, color);
}

void draw_ortho_quad(Texture *pb, Quad *quad, uint32_t color)
{
    Triangle t1 = {vec2_create(quad->p1.x, quad->p1.y), vec2_create(quad->p2.x, quad->p2.y), vec2_create(quad->p3.x, quad->p3.y)};
    Triangle t2 = {vec2_create(quad->p1.x, quad->p1.y), vec2_create(quad->p3.x, quad->p3.y), vec2_create(quad->p4.x, quad->p4.y)};

    draw_triangle(pb, t1, color);
    draw_triangle(pb, t2, color);
}

// in this case the SFA is 2d vertices: x,y
void draw_points(Texture *pb, SFA *points, uint32_t color)
{
    for (int i = 0; i < points->length; i += 2)
    {
        int x = points->data[i];
        int y = points->data[i + 1];
        texture_set(pb, x, y, color);
    }
}

// in this case the SFA is 2d vertices: x,y,x,y
void draw_tris(Texture *pb, SFA *vertices, SU32A *indices, uint32_t color)
{
    for (int i = 0; i < indices->length; i += 3)
    {
        int idx1 = indices->data[i];
        int idx2 = indices->data[i + 1];
        int idx3 = indices->data[i + 2];

        Vec2 p1 = {vertices->data[idx1 * 2], vertices->data[idx1 * 2 + 1]};
        Vec2 p2 = {vertices->data[idx2 * 2], vertices->data[idx2 * 2 + 1]};
        Vec2 p3 = {vertices->data[idx3 * 2], vertices->data[idx3 * 2 + 1]};

        Triangle t = {p1, p2, p3};
        draw_triangle(pb, t, color);
    }
}

void draw_tris_with_colors(Texture *pb, SFA *vertices, SU32A *indices, SU32A *colors)
{
    for (int i = 0; i < indices->length; i += 3)
    {
        int idx1 = indices->data[i];
        int idx2 = indices->data[i + 1];
        int idx3 = indices->data[i + 2];

        Vec2 p1 = {vertices->data[idx1 * 2], vertices->data[idx1 * 2 + 1]};
        Vec2 p2 = {vertices->data[idx2 * 2], vertices->data[idx2 * 2 + 1]};
        Vec2 p3 = {vertices->data[idx3 * 2], vertices->data[idx3 * 2 + 1]};

        Triangle t = {p1, p2, p3};
        uint32_t color = colors->data[i / 3];

        // print the color
        draw_triangle(pb, t, color);
    }
}

/*
    vertices: x1a,y1a,z1a,x1b,y1b,z1b,x1c,y1c,z1c, x2a,y2a,z2a,x2b,y2b,z2b,x2c,y2c,z2c, ...
    verticies are in groups of 3 (a,b,c) for each triangle, and each vertex has x,y in screenspace and z is cam dist

    for compute cost savings we will first average the z values of the 3 vertices of each triangle
    when drawing a pixel we will sample the z buffer at that position
    if the z value of the pixel is less than the z value in the z buffer we will draw the pixel and update the z buffer
    else we will skip the pixel
*/
void draw_tris_with_colors_and_depth(
    Texture *pb,
    FTexture *z_buffer,
    SFA *vertices,
    SU32A *indices,
    SU32A *colors)
{
    for (int i = 0; i < indices->length; i += 3)
    {
        int idx1 = indices->data[i];
        int idx2 = indices->data[i + 1];
        int idx3 = indices->data[i + 2];

        Vec2 p1 = {vertices->data[idx1 * 3], vertices->data[idx1 * 3 + 1]};
        Vec2 p2 = {vertices->data[idx2 * 3], vertices->data[idx2 * 3 + 1]};
        Vec2 p3 = {vertices->data[idx3 * 3], vertices->data[idx3 * 3 + 1]};

        // skip if all the verts are off screen
        if (p1.x < 0 && p2.x < 0 && p3.x < 0)
        {
            continue;
        }
        if (p1.x >= pb->width && p2.x >= pb->width && p3.x >= pb->width)
        {
            continue;
        }
        if (p1.y < 0 && p2.y < 0 && p3.y < 0)
        {
            continue;
        }
        if (p1.y >= pb->height && p2.y >= pb->height && p3.y >= pb->height)
        {
            continue;
        }

        // skip if they are too close to the camera
        const float near = 40.0f;
        if (vertices->data[idx1 * 3 + 2] < near && vertices->data[idx2 * 3 + 2] < near && vertices->data[idx3 * 3 + 2] < near)
        {
            continue;
        }

        Triangle t = {p1, p2, p3};
        uint32_t color = colors->data[i / 3];

        // average the z values of the 3 vertices
        float z = (vertices->data[idx1 * 3 + 2] + vertices->data[idx2 * 3 + 2] + vertices->data[idx3 * 3 + 2]) / 3.0f;

        // draw the triangle
        // draw_triangle_centroid_z_per_pixel_z_check(pb, z_buffer, t, color, z);
        draw_triangle_scanline_constant_z(pb, z_buffer, t, color, z);
    }
}

// identical but puts the face index in the face buffer for each pixel
void draw_tris_with_colors_and_depth_with_face_buffer(
    Texture *pb,
    FTexture *z_buffer,
    Texture *face_buffer,
    SFA *vertices,
    SU32A *indices,
    SU32A *colors)
{
    for (int i = 0; i < indices->length; i += 3)
    {
        int idx1 = indices->data[i];
        int idx2 = indices->data[i + 1];
        int idx3 = indices->data[i + 2];

        Vec2 p1 = {vertices->data[idx1 * 3], vertices->data[idx1 * 3 + 1]};
        Vec2 p2 = {vertices->data[idx2 * 3], vertices->data[idx2 * 3 + 1]};
        Vec2 p3 = {vertices->data[idx3 * 3], vertices->data[idx3 * 3 + 1]};

        // skip if all the verts are off screen
        if (p1.x < 0 && p2.x < 0 && p3.x < 0)
        {
            continue;
        }
        if (p1.x >= pb->width && p2.x >= pb->width && p3.x >= pb->width)
        {
            continue;
        }
        if (p1.y < 0 && p2.y < 0 && p3.y < 0)
        {
            continue;
        }
        if (p1.y >= pb->height && p2.y >= pb->height && p3.y >= pb->height)
        {
            continue;
        }

        // skip if they are too close to the camera
        const float near = 40.0f;
        if (vertices->data[idx1 * 3 + 2] < near && vertices->data[idx2 * 3 + 2] < near && vertices->data[idx3 * 3 + 2] < near)
        {
            continue;
        }

        Triangle t = {p1, p2, p3};
        uint32_t color = colors->data[i / 3];

        // average the z values of the 3 vertices
        float z = (vertices->data[idx1 * 3 + 2] + vertices->data[idx2 * 3 + 2] + vertices->data[idx3 * 3 + 2]) / 3.0f;

        // draw the triangle
        uint32_t face = i / 3;
        draw_triangle_scanline_constant_z_with_face_buffer(pb, z_buffer, t, color, z, face_buffer, face);
    }
}

void draw_tris_textured(
    Texture *pb,
    Texture *texture,
    FTexture *z_buffer,
    SFA *vertices, // x,y,w,x,y,w,x,y,w
    SU32A *indices,
    SFA *texcoords,
    SU32A *texcoord_indices,
    SFA *normals, // one per face: x,y,z...
    Vec3 cam_dir)
{
    int num_faces = indices->length / 3;
    for (int face = 0; face < num_faces; face += 1)
    {
        // Vec3 n1 = {normals->data[face], normals->data[face + 1], normals->data[face + 2]};
        // // if facing away from camera skip
        // if (vec3_dot(n1, cam_dir) > 0)
        // {
        //     continue;
        // }

        int idx1 = indices->data[face * 3];
        int idx2 = indices->data[face * 3 + 1];
        int idx3 = indices->data[face * 3 + 2];

        Vec2 p1 = {vertices->data[idx1 * 3], vertices->data[idx1 * 3 + 1]};
        Vec2 p2 = {vertices->data[idx2 * 3], vertices->data[idx2 * 3 + 1]};
        Vec2 p3 = {vertices->data[idx3 * 3], vertices->data[idx3 * 3 + 1]};

        // skip if all the verts are off screen
        if (p1.x < 0 && p2.x < 0 && p3.x < 0)
        {
            continue;
        }
        if (p1.x >= pb->width && p2.x >= pb->width && p3.x >= pb->width)
        {
            continue;
        }
        if (p1.y < 0 && p2.y < 0 && p3.y < 0)
        {
            continue;
        }
        if (p1.y >= pb->height && p2.y >= pb->height && p3.y >= pb->height)
        {
            continue;
        }

        // skip if they are too close to the camera
        const float near = 40.0f;
        if (vertices->data[idx1 * 3 + 2] < near && vertices->data[idx2 * 3 + 2] < near && vertices->data[idx3 * 3 + 2] < near)
        {
            continue;
        }

        Triangle t = {p1, p2, p3};

        // get the uv coordinates for the 3 vertices
        int uv_idx1 = texcoord_indices->data[face * 3];
        int uv_idx2 = texcoord_indices->data[face * 3 + 1];
        int uv_idx3 = texcoord_indices->data[face * 3 + 2];

        Vec2 uv1 = {texcoords->data[uv_idx1 * 2], texcoords->data[uv_idx1 * 2 + 1]};
        Vec2 uv2 = {texcoords->data[uv_idx2 * 2], texcoords->data[uv_idx2 * 2 + 1]};
        Vec2 uv3 = {texcoords->data[uv_idx3 * 2], texcoords->data[uv_idx3 * 2 + 1]};

        Triangle t_uv = {uv1, uv2, uv3};

        // average the z values of the 3 vertices
        float z = (vertices->data[idx1 * 3 + 2] + vertices->data[idx2 * 3 + 2] + vertices->data[idx3 * 3 + 2]) / 3.0f;
        // draw the triangle
        draw_triangle_scanline_with_texture(pb, texture, z_buffer, t, t_uv, z);
    }
}

// // in the center of each try in red just draw the face index
void draw_tris_face_numbers(Texture *pb, Texture *charmap, SFA *vertices, SU32A *indices, uint32_t size, uint32_t color)
{
    for (int i = 0; i < indices->length; i += 3)
    {
        int idx1 = indices->data[i];
        int idx2 = indices->data[i + 1];
        int idx3 = indices->data[i + 2];

        Vec2 p1 = {vertices->data[idx1 * 2], vertices->data[idx1 * 2 + 1]};
        Vec2 p2 = {vertices->data[idx2 * 2], vertices->data[idx2 * 2 + 1]};
        Vec2 p3 = {vertices->data[idx3 * 2], vertices->data[idx3 * 2 + 1]};

        Vec2 center = vec2_create((p1.x + p2.x + p3.x) / 3, (p1.y + p2.y + p3.y) / 3);
        char face_number[10];
        sprintf(face_number, "%d", i / 3);
        blit_string(pb, charmap, face_number, center.x, center.y, size, color);
    }
}

void draw_tris_with_colors_and_face_numbers(Texture *pb, Texture *charmap, SFA *vertices, SU32A *indices, SU32A *colors, uint32_t size, uint32_t color)
{
    for (int i = 0; i < indices->length; i += 3)
    {
        int idx1 = indices->data[i];
        int idx2 = indices->data[i + 1];
        int idx3 = indices->data[i + 2];

        Vec2 p1 = {vertices->data[idx1 * 2], vertices->data[idx1 * 2 + 1]};
        Vec2 p2 = {vertices->data[idx2 * 2], vertices->data[idx2 * 2 + 1]};
        Vec2 p3 = {vertices->data[idx3 * 2], vertices->data[idx3 * 2 + 1]};

        Triangle t = {p1, p2, p3};
        uint32_t face_color = colors->data[i / 3];
        draw_triangle(pb, t, face_color);

        Vec2 center = vec2_create((p1.x + p2.x + p3.x) / 3, (p1.y + p2.y + p3.y) / 3);
        char face_number[10];
        sprintf(face_number, "%d", i / 3);
        blit_string(pb, charmap, face_number, center.x, center.y, size, color);
    }
}

// in this case the SFA is 2d vertices: x,y
void draw_tris_lines(Texture *pb, SFA *vertices, SU32A *indices, uint32_t color)
{
    for (int i = 0; i < indices->length; i += 3)
    {
        int idx1 = indices->data[i];
        int idx2 = indices->data[i + 1];
        int idx3 = indices->data[i + 2];

        Vec2 p1 = {vertices->data[idx1 * 2], vertices->data[idx1 * 2 + 1]};
        Vec2 p2 = {vertices->data[idx2 * 2], vertices->data[idx2 * 2 + 1]};
        Vec2 p3 = {vertices->data[idx3 * 2], vertices->data[idx3 * 2 + 1]};

        Triangle t = {p1, p2, p3};
        draw_triangle_lines(pb, t, color);
    }
}

// in this case the SFA is 2d verticies with depth: x,y,_z do not use the z value
void draw_tris_lines_with_depth(Texture *pb, SFA *vertices, SU32A *indices, uint32_t color)
{
    for (int i = 0; i < indices->length; i += 3)
    {
        int idx1 = indices->data[i];
        int idx2 = indices->data[i + 1];
        int idx3 = indices->data[i + 2];

        Vec2 p1 = {vertices->data[idx1 * 3], vertices->data[idx1 * 3 + 1]};
        Vec2 p2 = {vertices->data[idx2 * 3], vertices->data[idx2 * 3 + 1]};
        Vec2 p3 = {vertices->data[idx3 * 3], vertices->data[idx3 * 3 + 1]};

        Triangle t = {p1, p2, p3};
        draw_triangle_lines(pb, t, color);
    }
}