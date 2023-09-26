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

#include "assets.h"

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

uint16_t old_palette[512];

int player_x  = 0;
int player_y  = 0;
int player_sx = 0;
int player_sy = 0;

int map_x = 0;
int map_y = 0;

bool is_running = true;

static void draw_map(int map_x, int map_y) {
    xm_setw(WR_INCR, 0x0001);
    xm_setw(WR_ADDR, START_A);
    for (int y = 0; y < HEIGHT_A; ++y) {
        for (int x = 0; x < WIDTH_A; ++x) {
            int addr = (y + map_y / 8) * MAP0_WIDTH + x + map_x / 8;
            uint16_t v = map0_data[addr] - 1;
            xm_setw(DATA, v);
        }
    }
}

static void init_bobs() {
        xm_setw(WR_INCR, 0x0001);
        xm_setw(WR_ADDR, START_BOBS);

        for (size_t i = 0; i < sizeof(bobs_bitmap) / sizeof(uint16_t); ++i)
            xm_setw(DATA, bobs_bitmap[i]);
}

static void wait_blit_ready() {
    uint16_t v;
    do {
      v = xm_getw(SYS_CTRL);
    } while ((v & 0x4000) != 0x0000);
}

static void wait_blit_done() {
    uint16_t v;
    do {
      v = xm_getw(SYS_CTRL);
    } while ((v & 0x2000) != 0x0000);
}

static void init_draw_ball() {
    xreg_setw(BLIT_MOD_S, 0x0000);
    xreg_setw(BLIT_MOD_D, WIDTH_B / 4 - 4);
    xreg_setw(BLIT_SHIFT, 0xFF00);
    xreg_setw(BLIT_LINES, 15);
}

static void draw_bob(uint16_t x, uint16_t y, int bob_index) {
    uint16_t shift = x & 0x3;
    if (bob_index >= 0) {
        xreg_setw(BLIT_SRC_S, START_BOBS + (bob_index * 4 * 16));
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

static void handle_event(io_event_t *io_event) {
    if (io_event->type == IO_KEYDOWN) {
        switch (io_event->scancode) {
            case IO_SCANCODE_UP:
                player_sy = -1;
                break;
            case IO_SCANCODE_DOWN:
                player_sy = 1;
                break;
            case IO_SCANCODE_LEFT:
                player_sx = -1;
                break;
            case IO_SCANCODE_RIGHT:
                player_sx = 1;
                break;
        }
    } else if (io_event->type == IO_KEYUP) {
        switch (io_event->scancode) {
            case IO_SCANCODE_UP:
                if (player_sy == -1)
                    player_sy = 0;
                break;
            case IO_SCANCODE_DOWN:
                if (player_sy == 1)
                    player_sy = 0;
                break;
            case IO_SCANCODE_LEFT:
                if (player_sx == -1)
                    player_sx = 0;
                break;
            case IO_SCANCODE_RIGHT:
                if (player_sx == 1)
                    player_sx = 0;
                break;
            case IO_SCANCODE_ESC:
                is_running = false;
                break;
        }
    }
}

static void wait_frame() {
    uint32_t f = XFrameCount;
    while (XFrameCount == f) {
        io_event_t io_event;
        if (receive_event(&io_event))
            handle_event(&io_event);
    }
}

static void save_palette() {
    xmem_getw_next_addr(XR_COLOR_ADDR);
    for (uint16_t i = 0; i < 512; i++)
        old_palette[i] = xmem_getw_next_wait();
}

static void restore_palette() {
    xmem_setw_next_addr(XR_COLOR_ADDR);
    for (uint16_t i = 0; i < 512; i++)
        xmem_setw_next(old_palette[i]);        // set palette data
}

// set first 16 colors to palette
static void set_palette() {
    // Playfield A
    xmem_setw_next_addr(XR_COLOR_ADDR);
    for (uint16_t i = 0; i < 16; i++)
        xmem_setw_next(tiles_palette[i]);
    
    // Playfield B
    xmem_setw_next_addr(XR_COLOR_ADDR + 0x100);
    for (uint16_t i = 0; i < 16; i++)
        xmem_setw_next(bobs_palette[i]);
}

void xosera_main() {
    init_io(true);

    xosera_init(0);

    install_intr();

    save_palette();
    set_palette();

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
    for (size_t i = 0; i < sizeof(tiles_bitmap) / sizeof(uint16_t); ++i)
        xm_setw(XDATA, tiles_bitmap[i]);

    init_bobs();

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

    int old_map_x = -1, old_map_y = -1;

    while (is_running) {
        wait_frame();

        if (map_x != old_map_x || map_y != old_map_y)
            draw_map(map_x, map_y);
        old_map_x = map_x;
        old_map_y = map_y;

        draw_bob(player_x - map_x, player_y - map_y, -1);

        player_x += player_sx;
        player_y += player_sy;

        if (player_x - map_x < 0) {
            if (map_x > 0) {
                map_x--;
            } else {
                player_x = 0;
            }
        }

        if (player_y - map_y < 0) {
            if (map_y > 0) {
                map_y--;
            } else {
                player_y = 0;
            }
        }

        if (player_x - map_x > WIDTH_B - 16) {
            if (map_x < MAP0_WIDTH * 8 - WIDTH_B) {
                map_x++;
            } else {
                player_x = MAP0_WIDTH * 8 - 16;
            }
        }

        if (player_y - map_y > HEIGHT_B - 16) {
            if (map_y < MAP0_HEIGHT * 8 - HEIGHT_B) {
                map_y++;
            } else {
                player_y = MAP0_HEIGHT * 8 - 16;
            }
        }

        draw_bob(player_x - map_x, player_y - map_y, 1);
    }

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