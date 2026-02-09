#ifndef SFA_H
#define SFA_H

#include "vec3.h"
#include "mat4.h"

// SFA: Sized Float Array
typedef struct
{
    int length;
    float *data;
} SFA;

SFA *sfa_new(int length);
void sfa_free(SFA *sfa);

// same for sized int array
// SIA: Sized Int Array
typedef struct
{
    int length;
    int *data;
} SIA;

SIA *sia_new(int length);
void sia_free(SIA *sia);

//////////////////////// TRANSFORM FUNCTIONS ////////////////////////
void sfa_rotate(SFA *sfa, Vec3 rotation);
void sfa_scale(SFA *sfa, Vec3 scale);
void sfa_translate(SFA *sfa, Vec3 translation);
void sfa_vec3_transform(SFA *sfa, Mat4 transform);
SFA *sfa_transform_vertices(const SFA *input_sfa, const Mat4 *mvp);

//////////////////////// PROJECTION FUNCTIONS ////////////////////////
SFA *sfa_orthographic_projection_xy(const SFA *sfa);
SFA *sfa_isometric_projection(const SFA *sfa);
SFA *sfa_perspective_project(const SFA *sfa);

//////////////////////// SHAPE FUNCTIONS ////////////////////////
SFA *create_x_axis_arrow(void);

#endif