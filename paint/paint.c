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

#include <io.h>

extern void install_intr(void);
extern void remove_intr(void);

extern volatile uint32_t XFrameCount;

#define WIDTH  40
#define HEIGHT 30

int mem[WIDTH][HEIGHT];

uint16_t tile_mem[] = {
    // 0
    0x8888,
    0x8888,
    0x8000,
    0x0008,
    0x8000,
    0x0008,
    0x8000,
    0x0008,
    0x8000,
    0x0008,
    0x8000,
    0x0008,
    0x8000,
    0x0008,
    0x8888,
    0x8888,

    // 1
    0xffff,
    0xffff,
    0xffff,
    0xffff,
    0xffff,
    0xffff,
    0xffff,
    0xffff,
    0xffff,
    0xffff,
    0xffff,
    0xffff,
    0xffff,
    0xffff,
    0xffff,
    0xffff,

    // 1
    0xf000,
    0x0000,
    0xff00,
    0x0000,
    0xfff0,
    0x0000,
    0xffff,
    0x0000,
    0x0f00,
    0x0000,
    0x0f00,
    0x0000,
    0x00f0,
    0x0000,
    0x00f0,
    0x0000};

void clear()
{
    xm_setw(WR_INCR, 0x0001);
    xm_setw(WR_ADDR, 0x0000);

    for (int y = 0; y < HEIGHT; ++y)
        for (int x = 0; x < WIDTH; ++x)
        {
            xm_setw(DATA, 0x0000);
            mem[x][y] = 0;
        }
}

void draw_cursor(int x, int y, bool is_visible)
{
    xm_setw(WR_ADDR, y * WIDTH + x);
    xm_setw(DATA, is_visible ? 0x0002 : mem[x][y] ? 0x0001 : 0x0000);
}

void draw_pixel(int x, int y, bool is_visible)
{
    mem[x][y] = is_visible ? 1 : 0;
    xm_setw(WR_ADDR, y * WIDTH + x);
    xm_setw(DATA, is_visible ? 0x0001 : 0x0000);
}

void main()
{
    init_io();

    xosera_init(0);

    xreg_setw(PA_DISP_ADDR, 0x0000);
    xreg_setw(PA_LINE_ADDR, 0x0000);
    xreg_setw(PA_LINE_LEN, WIDTH);

    // Set to tiled 4-bpp + Hx2 + Vx2
    xreg_setw(PA_GFX_CTRL, 0x0015);

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

    clear();

    int  x = 0, y = 0;
    bool is_running = true;
    while (is_running)
    {
        io_event_t io_event;
        if (receive_event(&io_event))
        {
            if (io_event.type == IO_MOUSEMOTION)
            {
                draw_cursor(x, y, false);
                x += (int)io_event.mx;
                y -= (int)io_event.my;
                if (x < 0)
                    x = 0;
                if (y < 0)
                    y = 0;
                if (x >= WIDTH)
                    x = WIDTH - 1;
                if (y >= HEIGHT)
                    y = HEIGHT - 1;
                if (io_event.mstat & 0x7)
                {
                    draw_pixel(x, y, io_event.mstat & 0x1);
                }
                draw_cursor(x, y, true);
            }
            else if (io_event.type == IO_KEYDOWN)
            {
                draw_cursor(x, y, false);

                switch (io_event.scancode)
                {
                    case IO_SCANCODE_UP:
                        y--;
                        break;
                    case IO_SCANCODE_DOWN:
                        y++;
                        break;
                    case IO_SCANCODE_LEFT:
                        x--;
                        break;
                    case IO_SCANCODE_RIGHT:
                        x++;
                        break;
                }
                draw_cursor(x, y, true);
            }
            else if (io_event.type == IO_KEYUP)
            {
                if (io_event.scancode == IO_SCANCODE_ESC)
                    is_running = false;
            }
        }
    }
}