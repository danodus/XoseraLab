/*
 * vim: set et ts=4 sw=4
 *------------------------------------------------------------
 *  __ __
 * |  |  |___ ___ ___ ___ ___
 * |-   -| . |_ -| -_|  _| .'|
 * |__|__|___|___|___|_| |__,|
 *
 * Xark's Open Source Enhanced Retro Adapter
 *
 * - "Not as clumsy or random as a GPU, an embedded retro
 *    adapter for a more civilized age."
 *
 * ------------------------------------------------------------
 * Portions Copyright (c) 2021 Daniel Cliche
 * Portions Copyright (c) 2021 Xark
 * MIT License
 *
 * Draw reference implementation
 * ------------------------------------------------------------
 */

#include <SDL.h>
#include <math.h>
#include <stdbool.h>
#include <stdio.h>

#include "../common/fx.h"

#define BLACK       0
#define RED         12
#define GREEN       10
#define BLUE        9
#define GRAY        7
#define DARK_GREEN  2
#define DARK_BLUE   1
#define DARK_YELLOW 14
#define WHITE       15

int screen_width  = 160;
int screen_height = 120;
int screen_scale  = 4;

int key_pressed = 0;

extern fx32 car_pos;
extern fx32 distance;
extern fx32 track_curvature;
extern fx32 curvature;

static SDL_Renderer * renderer;

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


void init();
void update(fx32 elapsed_time);

void draw_pixel(int x, int y, int color)
{
    uint16_t e     = defpal[color];
    int      red   = e >> 8;
    int      green = (e >> 4) & 0xF;
    int      blue  = e & 0xF;
    SDL_SetRenderDrawColor(renderer, red << 4, green << 4, blue << 4, SDL_ALPHA_OPAQUE);
    SDL_RenderDrawPoint(renderer, x, y);
}

void draw_sky()
{
    //
    // draw sky
    //

    for (int y = 0; y < screen_height / 2; y++)
        for (int x = 0; x < screen_width; x++)
            draw_pixel(x, y, y < screen_height / 4 ? DARK_BLUE : BLUE);

    for (int x = 0; x < screen_width; x++)
    {
        int hill_height = (int)(fabs(sinf(x * 0.01f + FLT(track_curvature)) * 16.0f));
        for (int y = screen_height / 2 - hill_height; y < screen_height / 2; y++)
            draw_pixel(x, y, DARK_YELLOW);
    }
}

void draw_terrain()
{
    //
    // draw terrain
    //

    for (int y = 0; y < screen_height / 2; y++)
    {
        fx32 yy           = FXI(y);
        fx32 perspective  = DIV(yy, FXI(screen_height / 2));
        fx32 middle_point = FX(0.5f) + FX(FLT(curvature) * powf((1.0f - FLT(perspective)), 3.0f));
        int  grass_color =
            sinf(20.0f * powf(1.0f - FLT(perspective), 3.0f) + FLT(distance) * 0.1f) > 0.0f ? GREEN : DARK_GREEN;
        int  clip_color = sinf(80.0f * powf(1.0f - FLT(perspective), 2.0f) + FLT(distance)) > 0.0f ? RED : WHITE;
        fx32 road_width = FX(0.1f) + MUL(perspective, FX(0.8f));

        fx32 clip_width = MUL(road_width, FX(0.15f));

        road_width = MUL(road_width, FX(0.5f));

        int left_grass  = MUL(middle_point - road_width - clip_width, FXI(screen_width));
        int left_clip   = MUL(middle_point - road_width, FXI(screen_width));
        int right_clip  = MUL(middle_point + road_width, FXI(screen_width));
        int right_grass = MUL(middle_point + road_width + clip_width, FXI(screen_width));

        for (int x = 0; x < screen_width; x++)
        {

            fx32 xx  = FXI(x);
            int  row = screen_height / 2 + y;

            if (xx >= 0 && xx < left_grass)
                draw_pixel(x, row, grass_color);
            if (xx >= left_grass && xx < left_clip)
                draw_pixel(x, row, clip_color);
            if (xx >= left_clip && xx < right_clip)
                draw_pixel(x, row, GRAY);
            if (xx >= right_clip && xx < right_grass)
                draw_pixel(x, row, clip_color);
            if (xx >= right_grass && xx < FXI(screen_width))
                draw_pixel(x, row, grass_color);
        }
    }
}

void draw_sprite_line(int x, int y, char * c, int color)
{
    for (int i = 0; *c != '\0'; ++i)
    {
        if (*c != ' ')
            draw_pixel(x, y, color);
        c++;
        x++;
    }
}

void draw_car()
{
    int x = screen_width / 2 + INT(DIV2(MUL(FXI(screen_width), car_pos), FX(2.0f)) - FXI(7));
    int y = screen_height - 20;

    draw_sprite_line(x, y++, "     x     ", BLACK);
    draw_sprite_line(x, y++, " xx xxx xx ", BLACK);
    draw_sprite_line(x, y++, " xxxxxxxxx ", BLACK);
    draw_sprite_line(x, y++, " xx xxx xx ", BLACK);
    draw_sprite_line(x, y++, "    xxx    ", BLACK);
    draw_sprite_line(x, y++, "xxx xxx xxx", BLACK);
    draw_sprite_line(x, y++, "xxxxxxxxxxx", BLACK);
    draw_sprite_line(x, y++, "xxx xxx xxx", BLACK);
}
int main(int argc, char * argv[])
{
    SDL_Init(SDL_INIT_VIDEO);

    SDL_Window * window = SDL_CreateWindow("Xosera Racing",
                                           SDL_WINDOWPOS_CENTERED_DISPLAY(1),
                                           SDL_WINDOWPOS_UNDEFINED,
                                           screen_width * screen_scale,
                                           screen_height * screen_scale,
                                           0);

    // SDL_SetWindowFullscreen(window, SDL_WINDOW_FULLSCREEN);

    renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_SOFTWARE);
    SDL_RenderSetScale(renderer, (float)screen_scale, (float)screen_scale);

    SDL_Event e;
    int       quit = 0;

    init();

    key_pressed = 1;        // temp

    // Uint32 last_ticks = SDL_GetTicks();
    while (!quit)
    {
        SDL_SetRenderDrawColor(renderer, 0, 0, 0, SDL_ALPHA_OPAQUE);
        SDL_RenderClear(renderer);

        // Uint32 ticks = SDL_GetTicks();

        // float delta_ticks = ((float)ticks - (float)last_ticks);

        // fx32 elapsed_time = FX(delta_ticks / 1000.0f);

        // if (delta_ticks < 16)
        //    SDL_Delay(16 - delta_ticks);

        // printf("%g\n", FLT(elapsed_time));

        // last_ticks = ticks;

        fx32 elapsed_time = FX(0.01666f);

        update(elapsed_time);

        draw_sky();
        draw_terrain();
        draw_car();

        SDL_RenderPresent(renderer);

        while (SDL_PollEvent(&e))
        {
            if (e.type == SDL_QUIT)
            {
                quit = 1;
            }
            else if (e.type == SDL_KEYDOWN)
            {
                switch (e.key.keysym.scancode)
                {
                    case SDL_SCANCODE_ESCAPE:
                        quit = 1;
                        break;
                    case SDL_SCANCODE_UP:
                        key_pressed |= 1;
                        break;
                    case SDL_SCANCODE_DOWN:
                        key_pressed |= 2;
                        break;
                    case SDL_SCANCODE_LEFT:
                        key_pressed |= 4;
                        break;
                    case SDL_SCANCODE_RIGHT:
                        key_pressed |= 8;
                        break;
                    default:
                        break;
                }
            }
            else if (e.type == SDL_KEYUP)
            {
                switch (e.key.keysym.scancode)
                {
                    case SDL_SCANCODE_UP:
                        key_pressed &= ~1;
                        break;
                    case SDL_SCANCODE_DOWN:
                        key_pressed &= ~2;
                        break;
                    case SDL_SCANCODE_LEFT:
                        key_pressed &= ~4;
                        break;
                    case SDL_SCANCODE_RIGHT:
                        key_pressed &= ~8;
                        break;
                    default:
                        break;
                }
            }
        }
    }

    SDL_DestroyWindow(window);
    SDL_Quit();

    return 0;
}
