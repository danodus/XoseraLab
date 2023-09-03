#ifndef GUI_H
#define GUI_H

typedef struct {
    int x, y;
} point_t;

typedef struct {
    int width, height;
} rsize_t;

typedef struct {
    point_t origin;
    rsize_t size;
} rect_t;

typedef struct {
    rect_t rect;
    
} view_t;
#endif