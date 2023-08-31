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


#define XR_TILE_ADDR 0x4000

#define WIDTH 640
#define NB_COLS (WIDTH / 8)

#define START_A     0
#define WIDTH_A     40
#define HEIGHT_A    30

#define START_B (START_A + (WIDTH_A * HEIGHT_A))
#define WIDTH_B 320
#define HEIGHT_B 240

#define START_BOBS (START_B + (WIDTH_B * HEIGHT_B / 4))

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
        0x8888, 0x8888,
        0x8000, 0x0008,
        0x8002, 0x2008,
        0x8002, 0x2008,
        0x8002, 0x2008,
        0x8002, 0x2008,
        0x8000, 0x0008,
        0x8888, 0x8888,
};

uint16_t ball_bob[] = {
// 0
        0x000f, 0xffff, 0xffff, 0xf000,
        0x0ff1, 0x1111, 0x1111, 0x1ff0,
        0xf111, 0xf111, 0x1111, 0x111f,
        0xf11f, 0x1111, 0x1111, 0x111f,
        0xf111, 0x1111, 0x1111, 0x111f,
        0xf111, 0x1111, 0x1111, 0x111f,
        0xf111, 0x1111, 0x1111, 0x111f,
        0xf111, 0x1111, 0x1111, 0x111f,
// 1
        0xf111, 0x1111, 0x1111, 0x111f,
        0xf111, 0x1111, 0x1111, 0x111f,
        0xf111, 0x1111, 0x1111, 0x111f,
        0xf111, 0x1111, 0x1111, 0x111f,
        0xf111, 0x1111, 0x1111, 0x111f,
        0xf111, 0x1111, 0x1111, 0x111f,
        0x0ff1, 0x1111, 0x1111, 0x1ff0,
        0x000f, 0xffff, 0xffff, 0xf000
};

uint8_t map[80*30] = {
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    0,1,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0
};

uint16_t old_palette[512];

void draw_background() {
    xm_setw(WR_INCR, 0x0001);
    xm_setw(WR_ADDR, START_A);
    for (int y = 0; y < HEIGHT_A; ++y) {
        for (int x = 0; x < WIDTH_A; ++x) {
            xm_setw(DATA, 0x0000);
        }
    }
}

void init_bobs() {
        xm_setw(WR_INCR, 0x0001);
        xm_setw(WR_ADDR, START_BOBS);

        for (size_t i = 0; i < sizeof(ball_bob) / sizeof(uint16_t); ++i)
            xm_setw(DATA, ball_bob[i]);
}

void wait_blit_ready() {
    uint16_t v;
    do {
      v = xm_getw(SYS_CTRL);
    } while ((v & 0x4000) != 0x0000);
}

void wait_blit_done() {
    uint16_t v;
    do {
      v = xm_getw(SYS_CTRL);
    } while ((v & 0x2000) != 0x0000);
}

void init_draw_ball() {
    xreg_setw(BLIT_MOD_S, 0x0000);
    xreg_setw(BLIT_MOD_D, WIDTH_B / 4 - 4);
    xreg_setw(BLIT_SHIFT, 0xFF00);
    xreg_setw(BLIT_LINES, 15);
}

void draw_ball(uint16_t x, uint16_t y, bool visible) {
    uint16_t shift = x & 0x3;
    if (visible) {
        xreg_setw(BLIT_SRC_S, START_BOBS);
        xreg_setw(BLIT_CTRL, 0x0010);
    } else {
        xreg_setw(BLIT_SRC_S, 0x0000);
        xreg_setw(BLIT_CTRL, 0x0001);
    }

    xreg_setw(BLIT_MOD_S, shift > 0 ? 0xFFFF : 0x0000);
    xreg_setw(BLIT_MOD_D, WIDTH_B / 4 - 4 - (shift > 0 ? 1 : 0));
    xreg_setw(BLIT_DST_D, START_B + y * WIDTH_B / 4 + (x >> 2));
    uint16_t mask_shift[] = {0xFF00, 0x7801, 0x3C02, 0x1E03};
    xreg_setw(BLIT_SHIFT, mask_shift[shift]);
    xreg_setw(BLIT_LINES, 15);
    wait_blit_ready();
    xreg_setw(BLIT_WORDS, 3 + (shift > 0 ? 1 : 0));
}

void wait_frame()
{
    uint32_t f = XFrameCount;
    while (XFrameCount == f)
        ;
}

void save_palette()
{
    for (uint16_t i = 0; i < 512; i++)
    {
        xm_setw(RD_XADDR, XR_COLOR_ADDR | i);
        old_palette[i] = xm_getw(XDATA); 
    }
}

void restore_palette()
{
    xmem_setw_next_addr(XR_COLOR_ADDR);
    for (uint16_t i = 0; i < 512; i++)
        xmem_setw_next(old_palette[i]);        // set palette data
}

// set first 16 colors to default VGA colors
static void set_default_colors()
{
    static const uint16_t def_colors16_a[16] = {0x0000,         // black
                                              0x000a,         // blue
                                              0x00a0,         // green
                                              0x00aa,         // cyan
                                              0x0a00,         // red
                                              0x0a0a,         // magenta
                                              0x0a50,         // brown
                                              0x0aaa,         // white
                                              0x0555,         // gray
                                              0x055f,         // light blue
                                              0x05f5,         // light green
                                              0x05ff,         // light cyan
                                              0x0f55,         // light red
                                              0x0f5f,         // light magenta
                                              0x0ff5,         // yellow
                                              0x0fff};        // bright white

    static const uint16_t def_colors16_b[16] = {0x0000,         // black
                                              0xf00a,         // blue
                                              0xf0a0,         // green
                                              0xf0aa,         // cyan
                                              0xfa00,         // red
                                              0xfa0a,         // magenta
                                              0xfa50,         // brown
                                              0xfaaa,         // white
                                              0xf555,         // gray
                                              0xf55f,         // light blue
                                              0xf5f5,         // light green
                                              0xf5ff,         // light cyan
                                              0xff55,         // light red
                                              0xff5f,         // light magenta
                                              0xfff5,         // yellow
                                              0xffff};        // bright white

    // Playfield A
    xmem_setw_next_addr(XR_COLOR_ADDR);
    for (uint16_t i = 0; i < 16; i++)
        xmem_setw_next(def_colors16_a[i]);
    
    // Playfield B
    xmem_setw_next_addr(XR_COLOR_ADDR + 0x100);
    for (uint16_t i = 0; i < 16; i++)
        xmem_setw_next(def_colors16_b[i]);
}

void xosera_demo()
{

    xosera_init(0);

    install_intr();

    save_palette();
    set_default_colors();

    // playfield A

    uint16_t old_pa_disp_addr = xreg_getw(PA_DISP_ADDR);
    xreg_setw(PA_DISP_ADDR, START_A);
    uint16_t old_pa_line_len = xreg_getw(PA_LINE_LEN);
    xreg_setw(PA_LINE_LEN, WIDTH_A);

    // set to tiled 4-bpp, Hx2, Vx2
    uint16_t old_pa_gfx_ctrl = xreg_getw(PA_GFX_CTRL);
    xreg_setw(PA_GFX_CTRL, 0x0015);

    // tile height to 8
    uint16_t old_pa_tile_ctrl = xreg_getw(PA_TILE_CTRL);
    xreg_setw(PA_TILE_CTRL, 0x0007);

    // set tiles
    xm_setw(WR_XADDR, XR_TILE_ADDR);
    for (size_t i = 0; i < sizeof(tile_mem) / sizeof(uint16_t); ++i)
        xm_setw(XDATA, tile_mem[i]);

    init_bobs();

    draw_background(1);

    // playfield B
    uint16_t old_pb_disp_addr = xreg_getw(PB_DISP_ADDR);
    xreg_setw(PB_DISP_ADDR, START_B);
    uint16_t old_pb_line_len = xreg_getw(PB_LINE_LEN);
    xreg_setw(PB_LINE_LEN, WIDTH_B / 4);

    // set to bitmap 4-bpp, Hx2, Vx2
    uint16_t old_pb_gfx_ctrl = xreg_getw(PB_GFX_CTRL);
    xreg_setw(PB_GFX_CTRL, 0x0055);

    // clear playfield B

    xreg_setw(BLIT_CTRL, 0x0001);
    xreg_setw(BLIT_SRC_S, 0x0000);
    xreg_setw(BLIT_MOD_D, 0x0000);
    xreg_setw(BLIT_DST_D, START_B);
    xreg_setw(BLIT_SHIFT, 0xFF00);
    xreg_setw(BLIT_LINES, HEIGHT_B - 1);
    xreg_setw(BLIT_WORDS, WIDTH_B / 4 - 1);
    wait_blit_done();

    init_draw_ball();

    draw_background();
    int x  = 0;
    int y  = 0;
    int sx = 1;
    int sy = 1;

    while (!checkchar())
    {
        wait_frame();
        draw_ball(x, y, false);

        x += sx;
        y += sy;
        if (x < 0)
        {
            x  = 0;
            sx = -sx;
        }
        if (y < 0)
        {
            y  = 0;
            sy = -sy;
        }
        if (x >= WIDTH_B - 16)
        {
            x  = WIDTH_B - 1 - 16;
            sx = -sx;
        }
        if (y >= HEIGHT_B - 16)
        {
            y  = HEIGHT_B - 1 - 16;
            sy = -sy;
        }
        draw_ball(x, y, true);
    }
    readchar();

    // restore Xosera registers
    xreg_setw(PB_GFX_CTRL, old_pb_gfx_ctrl);
    xreg_setw(PB_LINE_LEN, old_pb_line_len);
    xreg_setw(PB_DISP_ADDR, old_pb_disp_addr);
    xreg_setw(PA_TILE_CTRL, old_pa_tile_ctrl);
    xreg_setw(PA_GFX_CTRL, old_pa_gfx_ctrl);
    xreg_setw(PA_LINE_LEN, old_pa_line_len);
    xreg_setw(PA_DISP_ADDR, old_pa_disp_addr);

    restore_palette();

    remove_intr();
}