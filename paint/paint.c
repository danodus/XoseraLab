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
    0xffff
};

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

    xreg_setw(VID_RIGHT, 640);    

    // Set to tiled 4-bpp + Hx2 + Vx2
    xreg_setw(PA_GFX_CTRL, 0x0015);

    // blank PB
    xreg_setw(PB_GFX_CTRL, 0x0080);

    // tile height to 8
    xreg_setw(PA_TILE_CTRL, 0x0007);

    xm_setw(WR_XADDR, XR_TILE_ADDR);
    for (size_t i = 0; i < 4096; ++i)
    {
        if (i < sizeof(tile_mem) / sizeof(uint16_t))
        {
            xm_setw(XDATA, tile_mem[i]);
        }
        else
        {
            xm_setw(XDATA, 0x0000);
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
                x += (int)io_event.mx;
                y -= (int)io_event.my;
                if (x < 0)
                    x = 0;
                if (y < 0)
                    y = 0;
                if (x >= 640)
                    x = 640 - 1;
                if (y >= 480)
                    y = 480 - 1;
                if (io_event.mstat & 0x7)
                {
                    draw_pixel(x >> 4, y >> 4, io_event.mstat & 0x1);
                }
                xreg_setw(POINTER_H, x + 155);
                xreg_setw(POINTER_V, 0xF000 | y);    
            }
            else if (io_event.type == IO_KEYDOWN)
            {
                switch (io_event.scancode)
                {
                    case IO_SCANCODE_UP:
                        y-=1;
                        break;
                    case IO_SCANCODE_DOWN:
                        y+=1;
                        break;
                    case IO_SCANCODE_LEFT:
                        x-=1;
                        break;
                    case IO_SCANCODE_RIGHT:
                        x+=1;
                        break;
                    case IO_SCANCODE_A:
                        x=155;
                        y=525;
                        break;
                }
                xreg_setw(POINTER_H, x + 155);
                xreg_setw(POINTER_V, 0xF000 | y);                 
            }
            else if (io_event.type == IO_KEYUP)
            {
                if (io_event.scancode == IO_SCANCODE_ESC)
                    is_running = false;
            }
        }
    }
}