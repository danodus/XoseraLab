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

#include "sw_rasterizer.h"

#include <stdbool.h>

static draw_pixel_fn_t g_draw_pixel_fn;

void sw_init_rasterizer(draw_pixel_fn_t draw_pixel_fn)
{
    g_draw_pixel_fn = draw_pixel_fn;
}

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wuninitialized"
#pragma GCC diagnostic ignored "-Wmaybe-uninitialized"

// ----------------------------------------------------------------------------
// Line

typedef enum
{
    LINE_IDLE,
    LINE_INIT_0,
    LINE_INIT_1,
    LINE_DRAW
} line_states_t;

typedef struct
{
    bool          right;
    int           err;
    int           dx, dy;
    bool          movx, movy;
    line_states_t state;
} line_ctx_t;

static void process_line(line_ctx_t * ctx,
                         bool         reset_i,
                         bool         start_i,
                         bool         oe_i,
                         int          x0_i,
                         int          y0_i,
                         int          x1_i,
                         int          y1_i,
                         int *        x_o,
                         int *        y_o,
                         bool *       drawing_o,
                         bool *       busy_o,
                         bool *       done_o)
{
    ctx->movx = (2 * ctx->err >= ctx->dy);
    ctx->movy = (2 * ctx->err <= ctx->dx);

    switch (ctx->state)
    {
        case LINE_DRAW:
            if (oe_i)
            {
                if (*x_o == x1_i && *y_o == y1_i)
                {
                    ctx->state = LINE_IDLE;
                    *busy_o    = false;
                    *done_o    = true;
                }
                else
                {
                    if (ctx->movx && ctx->movy)
                    {
                        *x_o     = ctx->right ? *x_o + 1 : *x_o - 1;
                        *y_o     = *y_o + 1;
                        ctx->err = ctx->err + ctx->dy + ctx->dx;
                    }
                    else if (ctx->movx)
                    {
                        *x_o     = ctx->right ? *x_o + 1 : *x_o - 1;
                        ctx->err = ctx->err + ctx->dy;
                    }
                    else if (ctx->movy)
                    {
                        *y_o     = *y_o + 1;        // always down
                        ctx->err = ctx->err + ctx->dx;
                    }
                }
            }
            break;
        case LINE_INIT_0:
            ctx->state = LINE_INIT_1;
            ctx->dx    = ctx->right ? x1_i - x0_i : x0_i - x1_i;        // dx = abs(x1_i - x0_i)
            ctx->dy    = y0_i - y1_i;                                   // dy = y0_i - y1_i
            break;
        case LINE_INIT_1:
            ctx->state = LINE_DRAW;
            ctx->err   = ctx->dx + ctx->dy;
            *x_o       = x0_i;
            *y_o       = y0_i;
            break;
        default: {        // IDLE
            *done_o = false;
            if (start_i)
            {
                ctx->state = LINE_INIT_0;
                ctx->right = (x0_i < x1_i);        // draw right to left?
                *busy_o    = true;
            }
        }
    }

    if (reset_i)
    {
        ctx->state = LINE_IDLE;
        *busy_o    = false;
        *done_o    = false;
    }

    *drawing_o = (ctx->state == LINE_DRAW && oe_i);
}

// ----------------------------------------------------------------------------
// Line 1D

typedef enum
{
    LINE_1D_IDLE,
    LINE_1D_DRAW
} line_1d_states_t;

typedef struct
{
    line_1d_states_t state;
} line_1d_ctx_t;

static void process_line_1d(line_1d_ctx_t * ctx,
                            bool            reset_i,
                            bool            start_i,
                            bool            oe_i,
                            int             x0_i,
                            int             x1_i,
                            int *           x_o,
                            bool *          drawing_o,
                            bool *          busy_o,
                            bool *          done_o)
{
    switch (ctx->state)
    {
        case LINE_1D_DRAW:
            if (oe_i)
            {
                if (*x_o == x1_i)
                {
                    ctx->state = LINE_1D_IDLE;
                    *busy_o    = false;
                    *done_o    = true;
                }
                else
                {
                    *x_o = *x_o + 1;
                }
            }
            break;
        default: {        // IDLE
            *done_o = false;
            if (start_i)
            {
                ctx->state = LINE_1D_DRAW;
                *x_o       = x0_i;
                *busy_o    = true;
            }
        }
    }

    if (reset_i)
    {
        ctx->state = LINE_1D_IDLE;
        *busy_o    = false;
        *done_o    = false;
    }

    *drawing_o = (ctx->state == LINE_1D_DRAW && oe_i);
}


// ----------------------------------------------------------------------------
// Triangle Fill

typedef enum
{
    TRIANGLE_FILL_IDLE,
    TRIANGLE_FILL_INIT_A,
    TRIANGLE_FILL_INIT_B0,
    TRIANGLE_FILL_INIT_B1,
    TRIANGLE_FILL_INIT_H,
    TRIANGLE_FILL_START_A,
    TRIANGLE_FILL_START_B,
    TRIANGLE_FILL_START_H,
    TRIANGLE_FILL_EDGE,
    TRIANGLE_FILL_H_LINE,
    TRIANGLE_FILL_DONE
} triangle_fill_states_t;

typedef struct
{
    // line coordinates
    int x0a, y0a, x1a, y1a, xa, ya;
    int x0b, y0b, x1b, y1b, xb, yb;
    int x0h, x1h, xh;

    // previous y-value for edges
    int prev_y;

    // previous x-values for horizontal line
    int prev_xa;
    int prev_xb;

    // line control signals
    bool oe_a, oe_b, oe_h;
    bool drawing_h;
    bool busy_a, busy_b, busy_h;
    bool b_edge;        // which B edge are we drawing?

    // pipeline completion signals to match coordinates
    bool busy_p1, done_p1;

    triangle_fill_states_t state;

    line_ctx_t    draw_edge_a_ctx, draw_edge_b_ctx;
    line_1d_ctx_t draw_h_line_ctx;

} triangle_fill_ctx_t;

static void process_triangle_fill(triangle_fill_ctx_t * ctx,
                                  bool                  reset_i,
                                  bool                  start_i,
                                  bool                  oe_i,
                                  int                   x0_i,
                                  int                   y0_i,
                                  int                   x1_i,
                                  int                   y1_i,
                                  int                   x2_i,
                                  int                   y2_i,
                                  int *                 x_o,
                                  int *                 y_o,
                                  bool *                drawing_o,
                                  bool *                busy_o,
                                  bool *                done_o)
{
    switch (ctx->state)
    {
        case TRIANGLE_FILL_INIT_A:
            ctx->state   = TRIANGLE_FILL_INIT_B0;
            ctx->x0a     = x0_i;
            ctx->y0a     = y0_i;
            ctx->x1a     = x2_i;
            ctx->y1a     = y2_i;
            ctx->prev_xa = x0_i;
            ctx->prev_xb = x0_i;
            break;
        case TRIANGLE_FILL_INIT_B0:
            ctx->state  = TRIANGLE_FILL_START_A;
            ctx->b_edge = false;
            ctx->x0b    = x0_i;
            ctx->y0b    = y0_i;
            ctx->x1b    = x1_i;
            ctx->y1b    = y1_i;
            ctx->prev_y = y0_i;
            break;
        case TRIANGLE_FILL_INIT_B1:
            ctx->state  = TRIANGLE_FILL_START_B;        // we don't need to start A again
            ctx->b_edge = true;
            ctx->x0b    = x1_i;
            ctx->y0b    = y1_i;
            ctx->x1b    = x2_i;
            ctx->y1b    = y2_i;
            ctx->prev_y = y1_i;
            break;
        case TRIANGLE_FILL_START_A:
            ctx->state = TRIANGLE_FILL_START_B;
            break;
        case TRIANGLE_FILL_START_B:
            ctx->state = TRIANGLE_FILL_EDGE;
            break;
        case TRIANGLE_FILL_EDGE:
            if ((ctx->ya != ctx->prev_y || !ctx->busy_a) && (ctx->yb != ctx->prev_y || !ctx->busy_b))
            {
                ctx->state = TRIANGLE_FILL_START_H;
                ctx->x0h =
                    (ctx->prev_xa > ctx->prev_xb) ? ctx->prev_xb : ctx->prev_xa;        // always draw left to right
                ctx->x1h = (ctx->prev_xa > ctx->prev_xb) ? ctx->prev_xa : ctx->prev_xb;
            }
            break;
        case TRIANGLE_FILL_START_H:
            ctx->state = TRIANGLE_FILL_H_LINE;
            break;
        case TRIANGLE_FILL_H_LINE:
            if (!ctx->busy_h)
            {
                ctx->prev_y  = ctx->yb;        // safe to update previous values once h-line done
                ctx->prev_xa = ctx->xa;
                ctx->prev_xb = ctx->xb;
                if (!ctx->busy_b)
                {
                    ctx->state = (ctx->busy_a && ctx->b_edge == false) ? TRIANGLE_FILL_INIT_B1 : TRIANGLE_FILL_DONE;
                }
                else
                {
                    ctx->state = TRIANGLE_FILL_EDGE;
                }
            }
            break;
        case TRIANGLE_FILL_DONE:
            ctx->state   = TRIANGLE_FILL_IDLE;
            ctx->done_p1 = true;
            ctx->busy_p1 = false;
            break;
        default:        // IDLE
            if (start_i)
            {
                ctx->state   = TRIANGLE_FILL_INIT_A;
                ctx->busy_p1 = true;
            }
            ctx->done_p1 = false;
    }

    if (reset_i)
    {
        ctx->state   = TRIANGLE_FILL_IDLE;
        ctx->busy_p1 = false;
        ctx->done_p1 = false;
        ctx->b_edge  = false;
    }

    ctx->oe_a = (ctx->state == TRIANGLE_FILL_EDGE && ctx->ya == ctx->prev_y);
    ctx->oe_b = (ctx->state == TRIANGLE_FILL_EDGE && ctx->yb == ctx->prev_y);
    ctx->oe_h = oe_i;

    bool tmp_drawing_a, tmp_done_a;
    process_line(&ctx->draw_edge_a_ctx,
                 reset_i,
                 ctx->state == TRIANGLE_FILL_START_A,
                 ctx->oe_a,
                 ctx->x0a,
                 ctx->y0a,
                 ctx->x1a,
                 ctx->y1a,
                 &ctx->xa,
                 &ctx->ya,
                 &tmp_drawing_a,
                 &ctx->busy_a,
                 &tmp_done_a);

    bool tmp_drawing_b, tmp_done_b;
    process_line(&ctx->draw_edge_b_ctx,
                 reset_i,
                 ctx->state == TRIANGLE_FILL_START_B,
                 ctx->oe_b,
                 ctx->x0b,
                 ctx->y0b,
                 ctx->x1b,
                 ctx->y1b,
                 &ctx->xb,
                 &ctx->yb,
                 &tmp_drawing_b,
                 &ctx->busy_b,
                 &tmp_done_b);

    bool tmp_done_h;
    process_line_1d(&ctx->draw_h_line_ctx,
                    reset_i,
                    ctx->state == TRIANGLE_FILL_START_H,
                    ctx->oe_h,
                    ctx->x0h,
                    ctx->x1h,
                    &ctx->xh,
                    &ctx->drawing_h,
                    &ctx->busy_h,
                    &tmp_done_h);

    *x_o       = ctx->xh;
    *y_o       = ctx->prev_y;
    *drawing_o = ctx->drawing_h;
    *busy_o    = ctx->busy_p1;
    *done_o    = ctx->done_p1;
}

static void swapi(int * a, int * b)
{
    int c = *a;
    *a    = *b;
    *b    = c;
}

void sw_draw_line(int x0, int y0, int x1, int y1, int color)
{
    if (y0 > y1)
    {
        swapi(&x0, &x1);
        swapi(&y0, &y1);
    }

    line_ctx_t ctx;
    int        x, y;
    bool       drawing, busy, done;

    // Reset
    process_line(&ctx, true, false, false, x0, y0, x1, y1, &x, &y, &drawing, &busy, &done);

    bool start = true;
    while (!done)
    {
        process_line(&ctx, false, start, true, x0, y0, x1, y1, &x, &y, &drawing, &busy, &done);
        start = false;
        if (drawing)
            (*g_draw_pixel_fn)(x, y, color);
    }
}

void sw_draw_triangle(int x0, int y0, int x1, int y1, int x2, int y2, int color)
{
    sw_draw_line(x0, y0, x1, y1, color);
    sw_draw_line(x1, y1, x2, y2, color);
    sw_draw_line(x2, y2, x0, y0, color);
}

void sw_draw_filled_triangle(int x0, int y0, int x1, int y1, int x2, int y2, int color)
{
    if (y0 > y2)
    {
        swapi(&x0, &x2);
        swapi(&y0, &y2);
    }

    if (y0 > y1)
    {
        swapi(&x0, &x1);
        swapi(&y0, &y1);
    }

    if (y1 > y2)
    {
        swapi(&x1, &x2);
        swapi(&y1, &y2);
    }

    triangle_fill_ctx_t ctx;
    int                 x, y;
    bool                drawing, busy, done;

    // Reset
    process_triangle_fill(&ctx, true, false, false, x0, y0, x1, y1, x2, y2, &x, &y, &drawing, &busy, &done);

    bool start = true;
    while (!done)
    {
        process_triangle_fill(&ctx, false, start, true, x0, y0, x1, y1, x2, y2, &x, &y, &drawing, &busy, &done);
        start = false;
        if (drawing)
            (*g_draw_pixel_fn)(x, y, color);
    }
}

void sw_draw_filled_rectangle(int x0, int y0, int x1, int y1, int color)
{
    sw_draw_filled_triangle(x0, y0, x1, y0, x0, y1, color);
    sw_draw_filled_triangle(x1, y0, x0, y1, x1, y1, color);
}

#pragma GCC diagnostic pop
