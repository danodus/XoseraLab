
#include <math.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <basicio.h>
#include <machine.h>

#ifndef M_PI
#define M_PI 3.14159265358979323846264338327950288
#endif

#define NB_RECTS     100
#define NB_TRIANGLES 50

#include "cube.h"
#include "draw_api.h"
#include <xosera_m68k_api.h>

//#include <io.h>

extern void install_intr(void);
extern void remove_intr(void);

const uint16_t defpal[16] = {
    0x0000,        // black
    0x000A,        // blue
    0x00A0,        // green
    0x00AA,        // cyan
    0x0A00,        // red
    0x0A0A,        // magenta
    0x0AA0,        // brown
    0x0AAA,        // light gray
    0x0555,        // dark gray
    0x055F,        // light blue
    0x05F5,        // light green
    0x05FF,        // light cyan
    0x0F55,        // light red
    0x0F5F,        // light magenta
    0x0FF5,        // yellow
    0x0FFF         // white
};

uint16_t pal[256][3];
uint16_t old_palette[256];

const uint32_t copper_list[] = {COP_WAIT_V(40),
                                COP_MOVER(0x0065, PA_GFX_CTRL),        // Set to 8-bpp + Hx2 + Vx2
                                COP_WAIT_V(440),
                                COP_MOVER(0x00D5, PA_GFX_CTRL),        // Set to blank
                                COP_END()};

model_t * cube_model;

void calc_palette_mono()
{
    for (uint16_t i = 0; i < 256; i++)
    {
        pal[i][0] = i >> 4;
        pal[i][1] = i >> 4;
        pal[i][2] = i >> 4;
    }
}

void set_palette(float value)
{
    for (uint16_t i = 0; i < 256; i++)
    {
        xm_setw(WR_XADDR, XR_COLOR_ADDR | i);

        uint16_t r = pal[i][0] * value;
        uint16_t g = pal[i][1] * value;
        uint16_t b = pal[i][2] * value;

        uint16_t c = (r << 8) | (g << 4) | b;
        xm_setw(XDATA, c);        // set palette data
    }
}

void save_palette()
{
    xmem_getw_next_addr(XR_COLOR_ADDR);
    for (uint16_t i = 0; i < 256; i++)
        old_palette[i] = xmem_getw_next_wait();
}

void restore_palette()
{
    xmem_setw_next_addr(XR_COLOR_ADDR);
    for (uint16_t i = 0; i < 256; i++)
        xmem_setw_next(old_palette[i]);        // set palette data
}

void xosera_demo()
{
    // allocations
    cube_model = load_cube();

    xosera_init(0);

    //init_io();

    install_intr();

    xm_setw(WR_XADDR, XR_COPPER_ADDR);
    uint16_t * wp = (uint16_t *)copper_list;
    for (uint8_t i = 0; i < sizeof(copper_list) / sizeof(uint32_t); i++)
    {
        xm_setw(XDATA, *wp++);
        xm_setw(XDATA, *wp++);
    }

    uint16_t old_pa_line_len = xreg_getw(PA_LINE_LEN);
    xreg_setw(PA_LINE_LEN, 160);

    uint16_t old_pa_gfx_ctrl = xreg_getw(PA_GFX_CTRL);

    xd_init(0, 320, 200, 8);

    save_palette();

    calc_palette_mono();
    set_palette(1.0f);

    // initialize swap
    xd_init_swap();

    // enable Copper
    xreg_setw(COPP_CTRL, 0x8000);

    float yaw = 0.0f, theta = 0.0f;

    // Projection matrix
    mat4x4 mat_proj = matrix_make_projection(320, 200, 60.0f);

    int  mouse_x = 0, mouse_y = 0;

    vec3d vec_camera = {FX(0.0f), FX(0.0f), FX(0.0f), FX(1.0f)};

    while (!checkchar())
    {
        xd_clear();

        //
        // camera
        //

        vec3d  vec_up         = {FX(0.0f), FX(1.0f), FX(0.0f), FX(1.0f)};
        vec3d  vec_target     = {FX(0.0f), FX(0.0f), FX(1.0f), FX(1.0f)};
        mat4x4 mat_camera_rot = matrix_make_rotation_y(yaw);
        vec3d  vec_look_dir   = matrix_multiply_vector(&mat_camera_rot, &vec_target);
        vec_target            = vector_add(&vec_camera, &vec_look_dir);

        mat4x4 mat_camera = matrix_point_at(&vec_camera, &vec_target, &vec_up);

        // make view matrix from camera
        mat4x4 mat_view = matrix_quick_inverse(&mat_camera);

        //
        // world
        //

        mat4x4 mat_rot_z = matrix_make_rotation_z(theta);
        mat4x4 mat_rot_x = matrix_make_rotation_x(theta);

        mat4x4 mat_trans = matrix_make_translation(FX(0.0f), FX(0.0f), FX(3.0f));
        mat4x4 mat_world;
        mat_world = matrix_make_identity();
        mat_world = matrix_multiply_matrix(&mat_rot_z, &mat_rot_x);
        mat_world = matrix_multiply_matrix(&mat_world, &mat_trans);

        // Draw cube
        draw_model(320, 200, &vec_camera, cube_model, &mat_world, &mat_proj, &mat_view, true, true);

        xd_swap(true);
/*
        io_event_t io_event;
        if (receive_event(&io_event))
        {
            if (io_event.type == IO_MOUSEMOTION)
            {
                mouse_x += (int)io_event.mx;
                mouse_y += (int)io_event.my;
            }
            else if (io_event.type == IO_KEYDOWN)
            {
                float elapsed_time = 0.1f;
                vec3d vec_forward  = vector_mul(&vec_look_dir, MUL(FX(2.0f), FX(elapsed_time)));
                switch (io_event.scancode)
                {
                    case IO_SCANCODE_UP:
                        vec_camera.y += MUL(FX(8.0f), FX(elapsed_time));
                        break;
                    case IO_SCANCODE_DOWN:
                        vec_camera.y -= MUL(FX(8.0f), FX(elapsed_time));
                        break;
                    case IO_SCANCODE_LEFT:
                        vec_camera.x -= MUL(FX(8.0f), FX(elapsed_time));
                        break;
                    case IO_SCANCODE_RIGHT:
                        vec_camera.x += MUL(FX(8.0f), FX(elapsed_time));
                        break;
                    case IO_SCANCODE_W:
                        vec_camera = vector_add(&vec_camera, &vec_forward);
                        break;
                    case IO_SCANCODE_S:
                        vec_camera = vector_sub(&vec_camera, &vec_forward);
                        break;
                    case IO_SCANCODE_A:
                        yaw -= 2.0f * elapsed_time;
                        break;
                    case IO_SCANCODE_D:
                        yaw += 2.0f * elapsed_time;
                        break;
                    default:
                        // do nothing
                        break;
                }
            }
            else if (io_event.type == IO_KEYUP)
            {
                if (io_event.scancode == IO_SCANCODE_ESC)
                    is_running = false;
            }
        }
*/        

        theta += 0.1f;
    }
    readchar();

    // disable Copper
    xreg_setw(COPP_CTRL, 0x0000);

    restore_palette();

    // restore Xosera registers
    xreg_setw(PA_GFX_CTRL, old_pa_gfx_ctrl);
    xreg_setw(PA_LINE_LEN, old_pa_line_len);

    remove_intr();
}
