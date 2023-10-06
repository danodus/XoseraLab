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

/*
static int8_t g_sin_data[256] = {
    0,           // 0
    3,           // 1
    6,           // 2
    9,           // 3
    12,          // 4
    15,          // 5
    18,          // 6
    21,          // 7
    24,          // 8
    27,          // 9
    30,          // 10
    33,          // 11
    36,          // 12
    39,          // 13
    42,          // 14
    45,          // 15
    48,          // 16
    51,          // 17
    54,          // 18
    57,          // 19
    59,          // 20
    62,          // 21
    65,          // 22
    67,          // 23
    70,          // 24
    73,          // 25
    75,          // 26
    78,          // 27
    80,          // 28
    82,          // 29
    85,          // 30
    87,          // 31
    89,          // 32
    91,          // 33
    94,          // 34
    96,          // 35
    98,          // 36
    100,         // 37
    102,         // 38
    103,         // 39
    105,         // 40
    107,         // 41
    108,         // 42
    110,         // 43
    112,         // 44
    113,         // 45
    114,         // 46
    116,         // 47
    117,         // 48
    118,         // 49
    119,         // 50
    120,         // 51
    121,         // 52
    122,         // 53
    123,         // 54
    123,         // 55
    124,         // 56
    125,         // 57
    125,         // 58
    126,         // 59
    126,         // 60
    126,         // 61
    126,         // 62
    126,         // 63
    127,         // 64
    126,         // 65
    126,         // 66
    126,         // 67
    126,         // 68
    126,         // 69
    125,         // 70
    125,         // 71
    124,         // 72
    123,         // 73
    123,         // 74
    122,         // 75
    121,         // 76
    120,         // 77
    119,         // 78
    118,         // 79
    117,         // 80
    116,         // 81
    114,         // 82
    113,         // 83
    112,         // 84
    110,         // 85
    108,         // 86
    107,         // 87
    105,         // 88
    103,         // 89
    102,         // 90
    100,         // 91
    98,          // 92
    96,          // 93
    94,          // 94
    91,          // 95
    89,          // 96
    87,          // 97
    85,          // 98
    82,          // 99
    80,          // 100
    78,          // 101
    75,          // 102
    73,          // 103
    70,          // 104
    67,          // 105
    65,          // 106
    62,          // 107
    59,          // 108
    57,          // 109
    54,          // 110
    51,          // 111
    48,          // 112
    45,          // 113
    42,          // 114
    39,          // 115
    36,          // 116
    33,          // 117
    30,          // 118
    27,          // 119
    24,          // 120
    21,          // 121
    18,          // 122
    15,          // 123
    12,          // 124
    9,           // 125
    6,           // 126
    3,           // 127
    0,           // 128
    -3,          // 129
    -6,          // 130
    -9,          // 131
    -12,         // 132
    -15,         // 133
    -18,         // 134
    -21,         // 135
    -24,         // 136
    -27,         // 137
    -30,         // 138
    -33,         // 139
    -36,         // 140
    -39,         // 141
    -42,         // 142
    -45,         // 143
    -48,         // 144
    -51,         // 145
    -54,         // 146
    -57,         // 147
    -59,         // 148
    -62,         // 149
    -65,         // 150
    -67,         // 151
    -70,         // 152
    -73,         // 153
    -75,         // 154
    -78,         // 155
    -80,         // 156
    -82,         // 157
    -85,         // 158
    -87,         // 159
    -89,         // 160
    -91,         // 161
    -94,         // 162
    -96,         // 163
    -98,         // 164
    -100,        // 165
    -102,        // 166
    -103,        // 167
    -105,        // 168
    -107,        // 169
    -108,        // 170
    -110,        // 171
    -112,        // 172
    -113,        // 173
    -114,        // 174
    -116,        // 175
    -117,        // 176
    -118,        // 177
    -119,        // 178
    -120,        // 179
    -121,        // 180
    -122,        // 181
    -123,        // 182
    -123,        // 183
    -124,        // 184
    -125,        // 185
    -125,        // 186
    -126,        // 187
    -126,        // 188
    -126,        // 189
    -126,        // 190
    -126,        // 191
    -127,        // 192
    -126,        // 193
    -126,        // 194
    -126,        // 195
    -126,        // 196
    -126,        // 197
    -125,        // 198
    -125,        // 199
    -124,        // 200
    -123,        // 201
    -123,        // 202
    -122,        // 203
    -121,        // 204
    -120,        // 205
    -119,        // 206
    -118,        // 207
    -117,        // 208
    -116,        // 209
    -114,        // 210
    -113,        // 211
    -112,        // 212
    -110,        // 213
    -108,        // 214
    -107,        // 215
    -105,        // 216
    -103,        // 217
    -102,        // 218
    -100,        // 219
    -98,         // 220
    -96,         // 221
    -94,         // 222
    -91,         // 223
    -89,         // 224
    -87,         // 225
    -85,         // 226
    -82,         // 227
    -80,         // 228
    -78,         // 229
    -75,         // 230
    -73,         // 231
    -70,         // 232
    -67,         // 233
    -65,         // 234
    -62,         // 235
    -59,         // 236
    -57,         // 237
    -54,         // 238
    -51,         // 239
    -48,         // 240
    -45,         // 241
    -42,         // 242
    -39,         // 243
    -36,         // 244
    -33,         // 245
    -30,         // 246
    -27,         // 247
    -24,         // 248
    -21,         // 249
    -18,         // 250
    -15,         // 251
    -12,         // 252
    -9,          // 253
    -6,          // 254
    -4,          // 255
};
*/

static void init_audio_sample(int8_t * samp, int bytesize)
{
    uint16_t audio_vaddr = 0x8000;
    xm_setw(SYS_CTRL, 0x000F);        // make sure no nibbles masked

    xm_setw(WR_INCR, 0x0001);            // set write increment
    xm_setw(WR_ADDR, audio_vaddr);        // set write address

    for (int i = 0; i < bytesize; i += 2)
    {
        xm_setbh(DATA, *samp++);
        xm_setbl(DATA, *samp++);
    }
}


static void play_audio_sample(int bytesize, int speed)
{
    uint16_t audio_vaddr = 0x8000;
    xreg_setw(AUD0_VOL, 0x0000);           // set volume to 0%
    xreg_setw(AUD0_PERIOD, 0x0000);        // 1000 clocks per each sample byte
    xreg_setw(AUD0_LENGTH, 0x0000);        // 1000 clocks per each sample byte
    xreg_setw(AUD_CTRL, 0x0001);           // enable audio DMA to start playing

    uint16_t p  = speed;
    uint8_t  lv = 0x40;
    uint8_t  rv = 0x40;

    xreg_setw(AUD0_VOL, lv << 8 | rv);                 // set left 100% volume, right 50% volume
    xreg_setw(AUD0_PERIOD, p);                         // 1000 clocks per each sample byte
    xreg_setw(AUD0_START, audio_vaddr);                // address in VRAM
    xreg_setw(AUD0_LENGTH, (bytesize / 2) - 1);        // length in words (256 8-bit samples)
}

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

inline void set_copper1(uint16_t i0) {
    xmem_setw_next(i0);
}

inline void set_copper2(uint16_t i0, uint16_t i1) {
    xmem_setw_next(i0);
    xmem_setw_next(i1);
}

void update_copper()
{
    wait_frame();
    // uint16_t addr = (screen_width / 2) * (screen_height / 2);
    uint32_t addr = screen_width;
    xmem_setw_next_addr(XR_COPPER_ADDR);
    for (int y = 0; y < screen_height / 2; ++y)
    {
        fx32 middle_point = MUL(curvature, perspective_table[y]);

        int offset = INT(MUL(middle_point, FXI(screen_width)));

        set_copper1(COP_VPOS(screen_height + y * 2));

        uint32_t ao = addr - offset;
        set_copper2(COP_MOVER((uint16_t)(ao / 4), PA_LINE_ADDR));
        addr += terrain_width;

        set_copper2(COP_MOVER((2 * (ao & 0x3)), PA_H_SCROLL));

        uint16_t clip_color = clip_table[y][INT(distance) % 4];
        set_copper2(COP_MOVI(clip_color, XR_COLOR_ADDR + WHITE));
    }
    set_copper1(COP_END());
}

void main()
{
#if ENABLE_IO
    init_io();
#endif

    xosera_init(0);
    install_intr();

    uint16_t old_pa_line_len = xreg_getw(PA_LINE_LEN);
    xreg_setw(PA_LINE_LEN, screen_width / 4);

    // Set to bitmap 4-bpp + Hx2 + Vx2
    uint16_t old_pa_gfx_ctrl = xreg_getw(PA_GFX_CTRL);
    xreg_setw(PA_GFX_CTRL, 0x0055);

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

    // uint16_t t1         = xm_getw(TIMER);

    //init_audio_sample(g_sin_data, sizeof(g_sin_data));

    int audio_speed = 500;

    while (checkchar())
        readchar();

    while (!checkchar()) {

        //wait_frame();
        
        //play_audio_sample(sizeof(g_sin_data), audio_speed);    
        audio_speed += 50;
        if (audio_speed > 1000)
            audio_speed = 500;

        fx32 elapsed_time = FX(0.01666f);

        // float delta_ticks  = ((float)t2 - (float)t1);
        // fx32  elapsed_time = FX(delta_ticks / 10000.0f);
        // t1                 = t2;
        update(elapsed_time);
        update_copper();
    }
    readchar();

    // disable Copper
    xreg_setw(COPP_CTRL, 0x0000);

    xreg_setw(PA_GFX_CTRL, old_pa_gfx_ctrl);
    xreg_setw(PA_LINE_LEN, old_pa_line_len);

    remove_intr();
}