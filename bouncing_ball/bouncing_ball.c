#include <math.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <basicio.h>
#include <machine.h>

#include <xosera_m68k_api.h>

extern void install_intr(void);
extern void remove_intr(void);

extern volatile uint32_t XFrameCount;

#define WIDTH   40
#define HEIGHT  30

uint16_t tile_mem[] = {
    // 0
    0x8888, 0x8888,
    0x8000, 0x0008,
    0x8000, 0x0008,
    0x8000, 0x0008,
    0x8000, 0x0008,
    0x8000, 0x0008,
    0x8000, 0x0008,
    0x8888, 0x8888,

    // 1
    0x000f, 0xffff,
    0x0ff1, 0x1111,
    0xf111, 0xf111,
    0xf11f, 0x1111,
    0xf111, 0x1111,
    0xf111, 0x1111,
    0xf111, 0x1111,
    0xf111, 0x1111,

    // 2
    0xffff, 0xf000,
    0x1111, 0x1ff0,
    0x1111, 0x111f,
    0x1111, 0x111f,
    0x1111, 0x111f,
    0x1111, 0x111f,
    0x1111, 0x111f,
    0x1111, 0x111f,

    // 3
    0xf111, 0x1111,
    0xf111, 0x1111,
    0xf111, 0x1111,
    0xf111, 0x1111,
    0xf111, 0x1111,
    0xf111, 0x1111,
    0x0ff1, 0x1111,
    0x000f, 0xffff,

    // 4
    0x1111, 0x111f,
    0x1111, 0x111f,
    0x1111, 0x111f,
    0x1111, 0x111f,
    0x1111, 0x111f,
    0x1111, 0x111f,
    0x1111, 0x1ff0,
    0xffff, 0xf000,

    // 5
    0x0001, 0x4000,
    0x0011, 0x4400,
    0x0111, 0x4440,
    0x1111, 0x4444,
    0x0001, 0x4000,
    0x0001, 0x4000,
    0x0001, 0x4000,
    0x0001, 0x4000,
};

void draw_background()
{
    xm_setw(WR_INCR, 0x0001);
    xm_setw(WR_ADDR, 0x0000);

    for (int y = 0; y < HEIGHT; ++y)
        for (int x = 0; x < WIDTH; ++x)
            xm_setw(DATA, 0x0000);
}

void draw_ball(int x, int y, bool is_visible)
{
    // Ball is tiles are #1-4
    // Attributes: XXXXHVII (X=color extend depending on BPP, H V flip, and extra index bits II)
    xm_setw(WR_INCR, 0x0001);
    xm_setw(WR_ADDR, y * WIDTH + x);
    if (is_visible) {
        xm_setw(DATA, 0x0001);
        xm_setw(DATA, 0x0002);
        xm_setw(WR_ADDR, (y + 1) * WIDTH + x);
        xm_setw(DATA, 0x0003);
        xm_setw(DATA, 0x0004);
    } else {
        xm_setw(DATA, 0x0000);
        xm_setw(DATA, 0x0000);
        xm_setw(WR_ADDR, (y + 1) * WIDTH + x);
        xm_setw(DATA, 0x0000);
        xm_setw(DATA, 0x0000);
    }
}

void draw_four_flipped_arrows(int x, int y)
{
    // Arrow is tile #5 (upward arrow, blue left, red right)
    // Attributes: XXXXHVII (X=color extend depending on BPP, H V flip, and extra index bits II)
    xm_setw(WR_INCR, 0x0001);
    xm_setw(WR_ADDR, y * WIDTH + x);
    xm_setw(DATA, 0x0005);
    xm_setw(DATA, 0x0805);  // flip H
    xm_setw(DATA, 0x0405);  // flip V
    xm_setw(DATA, 0x0C05);  // flip HV
}

void wait_frame()
{
    uint32_t f = XFrameCount;
    while (XFrameCount == f)
        ;
}

void xosera_demo()
{

    xosera_init(0);

    // Set the Xosera interrupt mask
    uint16_t sc = xm_getw(SYS_CTRL);
    xm_setw(SYS_CTRL, sc | 0x8);

    install_intr();

    xreg_setw(PA_DISP_ADDR, 0x0000);
    xreg_setw(PA_LINE_ADDR, 0x0000);
    xreg_setw(PA_LINE_LEN, WIDTH);

    // Set to tiled 4-bpp + Hx2 + Vx2
    xreg_setw(PA_GFX_CTRL, 0x0025);

    // tile height to 8
    xreg_setw(PA_TILE_CTRL, 0x0007);

    xm_setw(XR_ADDR, XR_TILE_MEM);
    for (size_t i = 0; i < 4096; ++i)
    {
        if (i < sizeof(tile_mem) / sizeof(uint16_t))
        {
            xm_setw(XR_DATA, tile_mem[i]);
        }
        else
        {
            xm_setw(XR_DATA, 0x0000);
        }
    }

    draw_background();

    int x = 0;
    int y = 0;
    int sx = 1;
    int sy = 1;

    while (1) {
        wait_frame();

        // Debug
        draw_four_flipped_arrows(0, 0);

        draw_ball(x, y, false);
        x += sx;
        y += sy;
        if (x < 0) {
            x = 0;
            sx = -sx;
        }
        if (y < 0) {
            y = 0;
            sy = -sy;
        }
        if (x >= WIDTH - 1) {
            x = WIDTH - 1 - 1;
            sx = -sx;
        }
        if (y >= HEIGHT - 1) {
            y = HEIGHT - 1 - 1;
            sy = -sy;
        }
        draw_ball(x, y, true);
    }
}