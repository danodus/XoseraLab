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

#include "draw_api.h"

#include "sw_rasterizer.h"

#include <xosera_m68k_api.h>

#include "draw.h"

extern volatile uint32_t XFrameCount;

static uint8_t  g_disp_buffer;
static uint16_t g_start_line;
static uint16_t g_width;
static uint16_t g_height;
static uint8_t  g_bpp;
static uint16_t g_first_disp_buffer_addr, g_second_disp_buffer_addr;
static uint16_t g_cur_draw_buffer_addr;

void xd_wait_done()
{
    xv_prep();
    uint8_t sys_ctrl;
    do
    {
        sys_ctrl = xm_getbl(SYS_CTRL);
    } while (sys_ctrl & 0x80);
}

void xd_wait_frame()
{
    uint32_t f = XFrameCount;
    while (XFrameCount == f)
        ;
}

static void draw_pixel(int x, int y, int color)
{
    xv_prep();
    uint16_t ux = x;
    uint16_t uy = y;
    uint8_t  uc = color;
    if (ux < g_width && uy < g_height)
    {
        uint16_t addr = g_cur_draw_buffer_addr + (uy * (g_width / 2)) + (ux / 2);
        xm_setw(WR_ADDR, addr);
        xm_setbl(SYS_CTRL, (ux & 1) ? 0x03 : 0x0C);
        xm_setbh(DATA, uc);
        xm_setbl(DATA, uc);
    }
}

void wait_blit_ready() {
    xv_prep();
    uint16_t v;
    do {
      v = xm_getw(SYS_CTRL);
    } while ((v & 0x4000) != 0x0000);
}

void wait_blit_done() {
    xv_prep();
    uint16_t v;
    do {
      v = xm_getw(SYS_CTRL);
    } while ((v & 0x2000) != 0x0000);
}

void xd_init(int start_line, int width, int height, int bpp)
{
    sw_init_rasterizer(draw_pixel);
    g_start_line    = start_line;
    g_width         = width;
    g_height        = height;
    g_bpp           = bpp;
}

void xd_init_swap()
{
    xv_prep();
    g_disp_buffer = 0;

    g_first_disp_buffer_addr  = g_start_line * g_width / 2;
    g_second_disp_buffer_addr = g_first_disp_buffer_addr + g_height * g_width / 2;

    xreg_setw(PA_DISP_ADDR, g_first_disp_buffer_addr);
    // xreg_setw(PA_LINE_ADDR, 0x0000);
    xd_wait_done();
    g_cur_draw_buffer_addr = g_second_disp_buffer_addr;
}

void xd_swap(bool is_vsync_enabled)
{
    xv_prep();
    xd_wait_done();
    if (is_vsync_enabled)
        xd_wait_frame();

    if (g_disp_buffer)
    {
        g_disp_buffer = 0;
        xreg_setw(PA_DISP_ADDR, g_first_disp_buffer_addr);
        g_cur_draw_buffer_addr = g_second_disp_buffer_addr;
    }
    else
    {
        g_disp_buffer = 1;
        xreg_setw(PA_DISP_ADDR, g_second_disp_buffer_addr);
        g_cur_draw_buffer_addr = g_first_disp_buffer_addr;
    }
}

uint16_t xd_swap_copper(bool is_vsync_enabled)
{
    xd_wait_done();
    if (is_vsync_enabled)
        xd_wait_frame();

    uint16_t addr;

    if (g_disp_buffer)
    {
        g_disp_buffer          = 0;
        addr                   = g_first_disp_buffer_addr;
        g_cur_draw_buffer_addr = g_second_disp_buffer_addr;
    }
    else
    {
        g_disp_buffer          = 1;
        addr                   = g_second_disp_buffer_addr;
        g_cur_draw_buffer_addr = g_first_disp_buffer_addr;
    }

    return addr;
}


void xd_clear()
{
    xv_prep();
    xreg_setw(BLIT_CTRL, 0x0001);
    xreg_setw(BLIT_SRC_S, 0x0000);
    xreg_setw(BLIT_MOD_D, 0x0000);
    xreg_setw(BLIT_DST_D, g_cur_draw_buffer_addr);
    xreg_setw(BLIT_SHIFT, 0xFF00);
    xreg_setw(BLIT_LINES, 200 - 1);
    xreg_setw(BLIT_WORDS, 320 / 2 - 1);
    wait_blit_done();    
}

void xd_finish()
{
}

void xd_draw_line(int x0, int y0, int x1, int y1, int color)
{
    xv_prep();
    sw_draw_line(x0, y0, x1, y1, color);
    xm_setbl(SYS_CTRL, 0x0F);
}

void xd_draw_triangle(int x0, int y0, int x1, int y1, int x2, int y2, int color)
{
    xv_prep();
    sw_draw_triangle(x0, y0, x1, y1, x2, y2, color);
    xm_setbl(SYS_CTRL, 0x0F);
}

void xd_draw_filled_triangle(int x0, int y0, int x1, int y1, int x2, int y2, int color)
{
    xv_prep();
    sw_draw_filled_triangle(x0, y0, x1, y1, x2, y2, color);
    xm_setbl(SYS_CTRL, 0x0F);
}

void xd_draw_filled_rectangle(int x0, int y0, int x1, int y1, int color)
{
    xv_prep();
    sw_draw_filled_rectangle(x0, y0, x1, y1, color);
    xm_setbl(SYS_CTRL, 0x0F);
}
