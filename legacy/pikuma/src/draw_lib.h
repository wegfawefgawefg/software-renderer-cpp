#ifndef DRAW_LIB_H
#define DRAW_LIB_H

#include <SDL2/SDL.h>

#include "texture.h"
#include "sfa.h"
#include "su32a.h"
#include "f_texture.h"

//////////////////////// PRIMITIVE DRAWING FUNCTIONS ////////////////////////
void draw_line(Texture *pb, int x0, int y0, int x1, int y1, uint32_t color);
void draw_lines(Texture *pb, SFA *points, uint32_t color);
void draw_rect(Texture *pb, int x, int y, int w, int h, uint32_t color);
void draw_rect_lines(Texture *pb, int x, int y, int w, int h, uint32_t color);
void draw_triangle_lines(Texture *pb, Triangle t, uint32_t color);
void draw_triangle(Texture *pb, Triangle t, uint32_t color);
void draw_grid(Texture *pb, IVec2 start, IVec2 end, int spacing, uint32_t color);
void draw_grid_dots(Texture *pb, IVec2 start, IVec2 end, int spacing, uint32_t color);
void draw_checkerboard(Texture *pb, IVec2 start, IVec2 end, int spacing, uint32_t color1, uint32_t color2);
void draw_cursor(Texture *pb, int x, int y, int size, uint32_t color);
void draw_arrow(Texture *pb, int x, int y, int size, uint32_t color);

//////////////////////// DRAW SFA FUNCTIONS ////////////////////////
void draw_points(Texture *pb, SFA *points, uint32_t color);
void draw_tris(Texture *pb, SFA *vertices, SU32A *indices, uint32_t color);
void draw_tris_lines(Texture *pb, SFA *vertices, SU32A *indices, uint32_t color);
void draw_tris_lines_with_depth(Texture *pb, SFA *vertices, SU32A *indices, uint32_t color);
void draw_tris_with_colors(Texture *pb, SFA *vertices, SU32A *indices, SU32A *colors);
void draw_tris_face_numbers(Texture *pb, Texture *charmap, SFA *vertices, SU32A *indices, uint32_t size, uint32_t color);
void draw_tris_with_colors_and_face_numbers(Texture *pb, Texture *charmap, SFA *vertices, SU32A *indices, SU32A *colors, uint32_t size, uint32_t color);
void draw_tris_with_colors_and_depth(Texture *pb, FTexture *z_buffer, SFA *vertices, SU32A *indices, SU32A *colors);

void draw_tris_with_colors_and_depth_with_face_buffer(
    Texture *pb,
    FTexture *z_buffer,
    Texture *face_buffer,
    SFA *vertices,
    SU32A *indices,
    SU32A *colors);
void draw_triangle_scanline_constant_z_with_face_buffer(
    Texture *pb,
    FTexture *z_buffer,
    Triangle t,
    uint32_t color,
    float z,
    Texture *face_buffer,
    uint32_t face);
void draw_triangle_scanline_with_texture(
    Texture *pb,
    Texture *texture,
    FTexture *z_buffer,
    Triangle t,
    Triangle t_uv,
    float z);

void draw_tris_textured(
    Texture *pb,
    Texture *texture,
    FTexture *z_buffer,
    SFA *vertices, // x,y,w,x,y,w,x,y,w
    SU32A *indices,
    SFA *texcoords,
    SU32A *texcoord_indices,
    SFA *normals, // one per face: x,y,z...
    Vec3 cam_dir);

//////////////////////// ORTHOGRAPHIC PROJECTION ////////////////////////
void draw_ortho_quad_lines(Texture *pb, Quad *quad, uint32_t color);
void draw_ortho_quad(Texture *pb, Quad *quad, uint32_t color);

#endif // DRAW_H
