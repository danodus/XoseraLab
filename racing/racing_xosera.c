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

#define ENABLE_IO 0

#define BLACK       0
#define RED         12
#define GREEN       10
#define BLUE        9
#define GRAY        7
#define DARK_GREEN  2
#define DARK_BLUE   1
#define DARK_YELLOW 14
#define WHITE       15

extern void install_intr(void);
extern void remove_intr(void);

extern volatile uint32_t XFrameCount;

const int screen_width  = 320;
const int screen_height = 240;
const int terrain_width = 3 * screen_width;

int key_pressed = 0;

fx32 perspective_table[120];
fx32 clip_table[120][4];

extern fx32 car_pos;
extern fx32 distance;
extern fx32 track_curvature;
extern fx32 curvature;

void init();
void update(fx32 elapsed_time);

void draw_pixel(int x, int y, int color)
{
    uint16_t ux = x;
    uint16_t uy = y;
    uint8_t  uc = color << 4 | color;
    if (ux < terrain_width && uy < screen_height)
    {
        uint16_t addr = (uy * (terrain_width / 4)) + (ux / 4);
        xm_setw(WR_ADDR, addr);
        xm_setbl(SYS_CTRL, 0x8 >> (ux & 0x3));
        xm_setbh(DATA, uc);
        xm_setbl(DATA, uc);
        xm_setbl(SYS_CTRL, 0x0F);
    }
}

void draw_terrain()
{
    //
    // draw terrain
    //

    for (int y = 0; y < screen_height / 2; y++)
    {
        float perspective  = (float)y / (screen_height / 2);
        float middle_point = 0.5f;
        int   grass_color  = GREEN;
        int   clip_color   = WHITE;
        float road_width   = 0.1f + perspective * 0.8f;

        float clip_width = road_width * 0.15f;

        road_width = road_width * 0.5f;

        int left_grass  = (middle_point - road_width - clip_width) * screen_width + screen_width;
        int left_clip   = (middle_point - road_width) * screen_width + screen_width;
        int right_clip  = (middle_point + road_width) * screen_width + screen_width;
        int right_grass = (middle_point + road_width + clip_width) * screen_width + screen_width;

        for (int x = 0; x < terrain_width; x++)
        {
            int row = y;

            if (x >= 0 && x < left_grass)
                draw_pixel(x, row, grass_color);
            if (x >= left_grass && x < left_clip)
                draw_pixel(x, row, clip_color);
            if (x >= left_clip && x < right_clip)
                draw_pixel(x, row, GRAY);
            if (x >= right_clip && x < right_grass)
                draw_pixel(x, row, clip_color);
            if (x >= right_grass && x < FXI(terrain_width))
                draw_pixel(x, row, grass_color);
        }
    }
}

void draw_sky()
{
    //
    // draw sky
    //

    for (int y = 0; y < screen_height / 2; y++)
        for (int x = 0; x < terrain_width; x++)
            draw_pixel(x, y, y < screen_height / 4 ? DARK_BLUE : BLUE);

    for (int x = 0; x < terrain_width; x++)
    {
        int hill_height = (int)(fabs(sinf(x * 0.01f + FLT(track_curvature)) * 16.0f));
        for (int y = screen_height / 2 - hill_height; y < screen_height / 2; y++)
            draw_pixel(x, y, DARK_YELLOW);
    }
}

void wait_frame()
{
    uint32_t f = XFrameCount;
    while (XFrameCount == f)
        ;
}

void update_copper()
{
    wait_frame();
    // uint16_t addr = (screen_width / 2) * (screen_height / 2);
    uint32_t addr = screen_width;
    xm_setw(XR_ADDR, XR_COPPER_ADDR);
    for (int y = 0; y < screen_height / 2; ++y)
    {
        fx32 middle_point = MUL(curvature, perspective_table[y]);

        int offset = INT(MUL(middle_point, FXI(screen_width)));

        uint32_t   i  = COP_WAIT_V(screen_height + y * 2);
        uint16_t * wp = (uint16_t *)&i;
        xm_setw(XR_DATA, *wp++);
        xm_setw(XR_DATA, *wp);

        uint32_t ao = addr - offset;
        i           = COP_MOVER((uint16_t)(ao / 4), PA_LINE_ADDR);
        wp          = (uint16_t *)&i;
        xm_setw(XR_DATA, *wp++);
        xm_setw(XR_DATA, *wp);
        addr += terrain_width;

        i  = COP_MOVER((2 * (ao & 0x3)) << 8, PA_HV_SCROLL);
        wp = (uint16_t *)&i;
        xm_setw(XR_DATA, *wp++);
        xm_setw(XR_DATA, *wp);

        uint16_t clip_color = clip_table[y][INT(distance) % 4];
        i                   = COP_MOVEP(clip_color, WHITE);
        wp                  = (uint16_t *)&i;
        xm_setw(XR_DATA, *wp++);
        xm_setw(XR_DATA, *wp);
    }
    uint32_t   i  = COP_END();
    uint16_t * wp = (uint16_t *)&i;
    xm_setw(XR_DATA, *wp++);
    xm_setw(XR_DATA, *wp);
}

void main()
{
#if ENABLE_IO
    init_io();
#endif

    xosera_init(0);
    install_intr();

    xreg_setw(PA_DISP_ADDR, 0x0000);
    xreg_setw(PA_LINE_ADDR, 0x0000);
    xreg_setw(PA_LINE_LEN, screen_width / 4);

    // Set to bitmap 4-bpp + Hx2 + Vx2
    xreg_setw(PA_GFX_CTRL, 0x0055);

    // Blank playfield B
    xreg_setw(PB_GFX_CTRL, 0x0080);

    init();

    for (int y = 0; y < screen_height / 2; ++y)
    {
        float perspective    = (float)y / (float)(screen_height / 2);
        perspective_table[y] = FX(powf((1.0f - perspective), 3.0f));
        for (int i = 0; i < 4; i++)
        {
            float f = (float)i / 4;
            clip_table[y][i] =
                sinf(80.0f * powf(1.0f - perspective, 2.0f) + (2.0f * 3.141592654f * f)) > 0.0f ? 0x0F00 : 0x0FFF;
        }
    }

    // draw_sky();
    draw_terrain();

    update_copper();

    // enable Copper
    xreg_setw(COPP_CTRL, 0x8000);


    key_pressed = 1;        // temp

    bool is_running = true;
    // uint16_t t1         = xm_getw(TIMER);

    while (is_running)
    {

        // uint16_t t2 = xm_getw(TIMER);

        fx32 elapsed_time = FX(0.01666f);
        // float delta_ticks  = ((float)t2 - (float)t1);
        // fx32  elapsed_time = FX(delta_ticks / 10000.0f);
        // t1                 = t2;
        update(elapsed_time);
        update_copper();

#if ENABLE_IO
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
#endif        // ENABLE_IO
    }

    // disable Copper
    xreg_setw(COPP_CTRL, 0x0000);
    remove_intr();
}