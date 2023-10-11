#include <math.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <machine.h>
#include <basicio.h>

#include <xosera_m68k_api.h>

#define GLITCHY 1

static uint16_t g_audio_vaddr[] = {0x8000, 0x9000, 0xA000, 0xB000, 0xC000, 0xD000};
#if GLITCHY
static int g_nb_cycles[] = {1,2,1,2,1,2};
static int g_audio_speed_inc = 0;
#else
static int g_nb_cycles[] = {1,1,1,1,1,1};
static int g_audio_speed_inc = 10;
#endif

#define NB_VADDR (sizeof(g_audio_vaddr) / sizeof(g_audio_vaddr[0]))

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

static void init_audio_sample(int8_t * samp, int bytesize)
{
    
    xm_setw(SYS_CTRL, 0x000F);        // make sure no nibbles masked

    xm_setw(WR_INCR, 0x0001);            // set write increment

    // clear memory with random data
    xm_setw(WR_ADDR, 0);        // set write address
    for (int i = 0; i < 128*1024/2; i++)
    {
        uint16_t v = rand() % 65536;
        xm_setw(DATA, v);
    }

    for (unsigned int a = 0; a < NB_VADDR; ++a) {
        xm_setw(WR_ADDR, g_audio_vaddr[a]);        // set write address
        for (int cycle = 0; cycle < g_nb_cycles[a]; ++cycle) {
            int8_t *s = samp;
            for (int i = 0; i < bytesize; i += 2)
            {
                xm_setbh(DATA, *s++);
                xm_setbl(DATA, *s++);
            }
        }
    }
}

static void play_audio_sample(unsigned int index, int bytesize, int speed)
{
    xreg_setw(AUD_CTRL, 0x0001);           // enable audio DMA to start playing

    uint16_t p  = speed;
    uint8_t  lv = 0x40;
    uint8_t  rv = 0x40;

    int size = bytesize * g_nb_cycles[index];

    xreg_setw(AUD0_VOL, lv << 8 | rv);                 // set left 100% volume, right 50% volume
    xreg_setw(AUD0_PERIOD, p);                // 1000 clocks per each sample byte
    xreg_setw(AUD0_LENGTH, (size / 2) - 1);            // length in words (256 8-bit samples)
    xreg_setw(AUD0_START, g_audio_vaddr[index]);         // address in VRAM
}

void main(void)
{
    xosera_init(0);

    init_audio_sample(g_sin_data, sizeof(g_sin_data));

    while(checkchar())
        readchar();

    int index = 0;
    int audio_speed = 500;
    while (!checkchar())
    {
        play_audio_sample(index, sizeof(g_sin_data), audio_speed);
        index = (index + 1) % NB_VADDR;
        audio_speed += g_audio_speed_inc;
        if (audio_speed < 500 || audio_speed > 1000)
            g_audio_speed_inc = -g_audio_speed_inc;
    }
    readchar();
}
