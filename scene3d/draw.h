/*
 * vim: set et ts=4 sw=4
 *------------------------------------------------------------
 *  __ __
 * |  |  |___ ___ ___ ___ ___
 * |-   -| . |_ -| -_|  _| .'|
 * |__|__|___|___|___|_| |__,|
 *
 * Xark's Open Source Enhanced Retro Adapter
 *
 * - "Not as clumsy or random as a GPU, an embedded retro
 *    adapter for a more civilized age."
 *
 * ------------------------------------------------------------
 * Portions Copyright (c) 2021 Daniel Cliche
 * Portions Copyright (c) 2021 Xark
 * MIT License
 *
 * Draw
 * ------------------------------------------------------------
 */

#ifndef DRAW_H
#define DRAW_H

#include <stdbool.h>
#include <stddef.h>

#define FIXED_POINT 1

#define _FRACTION_MASK(scale) (0xffffffff >> (32 - scale))
#define _WHOLE_MASK(scale)    (0xffffffff ^ FRACTION_MASK(scale))

#define _FLOAT_TO_FIXED(x, scale) ((x) * (float)(1 << scale))
#define _FIXED_TO_FLOAT(x, scale) ((float)(x) / (double)(1 << scale))
#define _INT_TO_FIXED(x, scale)   ((x) << scale)
#define _FIXED_TO_INT(x, scale)   ((x) >> scale)
#define _FRACTION_PART(x, scale)  ((x)&FRACTION_MASK(scale))
#define _WHOLE_PART(x, scale)     ((x)&WHOLE_MASK(scale))

//#define _MUL(x, y, scale) (((long long)(x) * (long long)(y)) >> scale)
#define _MUL(x, y, scale) ((int)((x) >> (scale / 2)) * (int)((y) >> (scale / 2)))

//#define _DIV(x, y, scale) (((long long)(x) << scale) / (y))
#define _DIV(x, y, scale)       (((int)(x) << (scale / 2)) / (int)((y) >> (scale / 2)))
#define _DIV2(x, y, scale, adj) (((int)(x) << (scale / 2 - (adj))) / (int)((y) >> (scale / 2 + (adj))))


#if FIXED_POINT

typedef int fx32;

#define SCALE 16

#define FX(x)  ((fx32)_FLOAT_TO_FIXED(x, SCALE))
#define INT(x) ((int)_FIXED_TO_INT(x, SCALE))
#define FLT(x) ((float)_FIXED_TO_FLOAT(x, SCALE))
#define MUL(x, y) _MUL(x, y, SCALE)
#define DIV(x, y)  _DIV(x, y, SCALE)
#define DIV2(x, y) _DIV2(x, y, SCALE, 2)

#define SIN(x)  FX(sinf(_FIXED_TO_FLOAT(x, SCALE)))
#define COS(x)  FX(cosf(_FIXED_TO_FLOAT(x, SCALE)))
#define TAN(x)  FX(tanf(_FIXED_TO_FLOAT(x, SCALE)))
#define SQRT(x) FX(sqrtf(_FIXED_TO_FLOAT(x, SCALE)))

#else

typedef float fx32;

#define FX(x)      (x)
#define INT(x)     ((int)(x))
#define FLT(x)     (x)
#define MUL(x, y)  ((x) * (y))
#define DIV(x, y)  ((x) / (y))
#define DIV2(x, y) DIV(x, y)

#define SIN(x)  (sinf(x))
#define COS(x)  (cosf(x))
#define TAN(x)  (tanf(x))
#define SQRT(x) (sqrtf(x))

#endif


typedef struct
{
    fx32 x, y, z, w;
} vec3d;

typedef struct
{
    fx32 m[4][4];
} mat4x4;

typedef struct
{
    int   indices[3];
    vec3d col;
} face_t;

typedef struct
{
    vec3d p[3];
    vec3d col;
} triangle_t;

typedef struct
{
    size_t   nb_vertices;
    size_t   nb_faces;
    vec3d *  vertices;
    face_t * faces;
} mesh_t;

typedef struct
{
    mesh_t mesh;

    // Internal buffers
    triangle_t * triangles_to_raster;
} model_t;

vec3d matrix_multiply_vector(mat4x4 * m, vec3d * i);
vec3d vector_add(vec3d * v1, vec3d * v2);
vec3d vector_sub(vec3d * v1, vec3d * v2);
vec3d vector_mul(vec3d * v1, fx32 k);
vec3d vector_div(vec3d * v1, fx32 k);
fx32  vector_dot_product(vec3d * v1, vec3d * v2);
fx32  vector_length(vec3d * v);
vec3d vector_normalize(vec3d * v);
vec3d vector_cross_product(vec3d * v1, vec3d * v2);

mat4x4 matrix_make_identity();
mat4x4 matrix_make_projection(int viewport_width, int viewport_height, float fov);
mat4x4 matrix_make_rotation_x(float theta);
mat4x4 matrix_make_rotation_y(float theta);
mat4x4 matrix_make_rotation_z(float theta);
mat4x4 matrix_make_translation(fx32 x, fx32 y, fx32 z);
mat4x4 matrix_multiply_matrix(mat4x4 * m1, mat4x4 * m2);
mat4x4 matrix_point_at(vec3d * pos, vec3d * target, vec3d * up);
mat4x4 matrix_quick_inverse(mat4x4 * m);

void draw_model(int       viewport_width,
                int       viewport_height,
                vec3d *   vec_camera,
                model_t * model,
                mat4x4 *  mat_world,
                mat4x4 *  mat_projection,
                mat4x4 *  mat_view,
                bool      is_lighting_ena,
                bool      is_wireframe);

#endif