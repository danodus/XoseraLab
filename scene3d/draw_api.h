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
 * Draw API
 * ------------------------------------------------------------
 */

#ifndef DRAW_API_H
#define DRAW_API_H

#include <stdbool.h>
#include <stdint.h>

void     xd_init(int start_line, int width, int height, int bpp);
void     xd_init_swap();
void     xd_swap(bool is_vsync_enabled);
uint16_t xd_swap_copper(bool is_vsync_enabled);
void     xd_clear();
void     xd_finish();

void xd_draw_line(int x0, int y0, int x1, int y1, int color);
void xd_draw_triangle(int x0, int y0, int x1, int y1, int x2, int y2, int color);
void xd_draw_filled_triangle(int x0, int y0, int x1, int y1, int x2, int y2, int color);
void xd_draw_filled_rectangle(int x0, int y0, int x1, int y1, int color);

#endif
