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

#include <fx.h>
#include <io.h>

extern void install_intr(void);
extern void remove_intr(void);

extern volatile uint32_t XFrameCount;

int screen_width  = 320;
int screen_height = 240;

int key_pressed = 0;

void init();
void update(fx32 elapsed_time);

void draw_pixel(int x, int y, int color)
{
    uint16_t ux = x;
    uint16_t uy = y;
    uint8_t  uc = color;
    if (ux < screen_width && uy < screen_height)
    {
        uint16_t addr = (uy * (screen_width / 2)) + (ux / 2);
        xm_setw(WR_ADDR, addr);
        xm_setbl(SYS_CTRL, (ux & 1) ? 0x03 : 0x0C);
        xm_setbh(DATA, uc);
        xm_setbl(DATA, uc);
        xm_setbl(SYS_CTRL, 0x0F);
    }
}

void main()
{
    init_io();

    xosera_init(0);

    xreg_setw(PA_DISP_ADDR, 0x0000);
    xreg_setw(PA_LINE_ADDR, 0x0000);
    xreg_setw(PA_LINE_LEN, screen_width / 2);

    // Set to bitmap 8-bpp + Hx2 + Vx2
    xreg_setw(PA_GFX_CTRL, 0x0065);

    init();

    key_pressed = 1;        // temp

    bool is_running = true;
    // uint16_t t1         = xm_getw(TIMER);
    while (is_running)
    {

        // uint16_t t2 = xm_getw(TIMER);

        fx32 elapsed_time = FX(0.01666f);
        // fx32 elapsed_time = FXI((t2 - t1) / 10);
        // t1                = t2;
        update(elapsed_time);

        io_event_t io_event;
        if (receive_event(&io_event))
        {
            if (io_event.type == IO_KEYDOWN)
            {
                switch (io_event.scancode)
                {
                    case IO_SCANCODE_UP:
                        key_pressed |= 1;
                        break;
                    case IO_SCANCODE_DOWN:
                        key_pressed |= 2;
                        break;
                    case IO_SCANCODE_LEFT:
                        key_pressed |= 4;
                        break;
                    case IO_SCANCODE_RIGHT:
                        key_pressed |= 8;
                        break;
                }
            }
            else if (io_event.type == IO_KEYUP)
            {
                switch (io_event.scancode)
                {
                    case IO_SCANCODE_ESC:
                        is_running = false;
                        break;
                    case IO_SCANCODE_UP:
                        key_pressed &= ~1;

                        break;
                    case IO_SCANCODE_DOWN:
                        key_pressed &= ~2;
                        break;
                    case IO_SCANCODE_LEFT:
                        key_pressed &= ~4;

                        break;
                    case IO_SCANCODE_RIGHT:
                        key_pressed &= ~8;
                        break;
                }
            }
        }
    }
}