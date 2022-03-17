
#include "cube.h"

#include <stdlib.h>

static vec3d vertices[] = {
    {FX(0.0f), FX(0.0f), FX(0.0f), FX(1.0f)},        // 0
    {FX(0.0f), FX(1.0f), FX(0.0f), FX(1.0f)},        // 1
    {FX(1.0f), FX(1.0f), FX(0.0f), FX(1.0f)},        // 2
    {FX(1.0f), FX(0.0f), FX(0.0f), FX(1.0f)},        // 3
    {FX(1.0f), FX(1.0f), FX(1.0f), FX(1.0f)},        // 4
    {FX(1.0f), FX(0.0f), FX(1.0f), FX(1.0f)},        // 5
    {FX(0.0f), FX(1.0f), FX(1.0f), FX(1.0f)},        // 6
    {FX(0.0f), FX(0.0f), FX(1.0f), FX(1.0f)}         // 7
};

face_t faces[] = {
    // South
    {{0, 1, 2}, {FX(1.0f), FX(1.0f), FX(1.0f), FX(1.0f)}},
    {{0, 2, 3}, {FX(1.0f), FX(1.0f), FX(1.0f), FX(1.0f)}},

    // East
    {{3, 2, 4}, {FX(1.0f), FX(1.0f), FX(1.0f), FX(1.0f)}},
    {{3, 4, 5}, {FX(1.0f), FX(1.0f), FX(1.0f), FX(1.0f)}},

    // North
    {{5, 4, 6}, {FX(1.0f), FX(1.0f), FX(1.0f), FX(1.0f)}},
    {{5, 6, 7}, {FX(1.0f), FX(1.0f), FX(1.0f), FX(1.0f)}},

    // West
    {{7, 6, 1}, {FX(1.0f), FX(1.0f), FX(1.0f), FX(1.0f)}},
    {{7, 1, 0}, {FX(1.0f), FX(1.0f), FX(1.0f), FX(1.0f)}},

    // Top
    {{1, 6, 4}, {FX(1.0f), FX(1.0f), FX(1.0f), FX(1.0f)}},
    {{1, 4, 2}, {FX(1.0f), FX(1.0f), FX(1.0f), FX(1.0f)}},

    // Bottom
    {{5, 7, 0}, {FX(1.0f), FX(1.0f), FX(1.0f), FX(1.0f)}},
    {{5, 0, 3}, {FX(1.0f), FX(1.0f), FX(1.0f), FX(1.0f)}}};

model_t * load_cube()
{
    model_t * model = malloc(sizeof(model_t));

    model->mesh.nb_faces       = sizeof(faces) / sizeof(face_t);
    model->mesh.nb_vertices    = sizeof(vertices) / sizeof(vec3d);
    model->mesh.faces          = faces;
    model->mesh.vertices       = vertices;
    model->triangles_to_raster = malloc(sizeof(triangle_t) * model->mesh.nb_faces);

    return model;
}
