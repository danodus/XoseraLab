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

#include <SDL2/SDL.h>
#include <stdbool.h>
#include <stdio.h>

#include "../common/fx.h"

int screen_width  = 160;
int screen_height = 120;
int screen_scale  = 4;

int key_pressed = 0;

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

    Uint32 last_ticks = SDL_GetTicks();
    while (!quit)
    {
        SDL_SetRenderDrawColor(renderer, 0, 0, 0, SDL_ALPHA_OPAQUE);
        SDL_RenderClear(renderer);

        Uint32 ticks = SDL_GetTicks();

        float delta_ticks = ((float)ticks - (float)last_ticks);

        fx32 elapsed_time = FX(delta_ticks / 1000.0f);

        if (delta_ticks < 16)
            SDL_Delay(16 - delta_ticks);

        // printf("%g\n", FLT(elapsed_time));

        last_ticks = ticks;

        update(elapsed_time);

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
