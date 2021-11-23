// Ref.: https://www.youtube.com/watch?v=KkMZI5Jbf18

#include "../common/fx.h"

#include <math.h>

#define BLACK       0
#define RED         12
#define GREEN       10
#define BLUE        9
#define GRAY        7
#define DARK_GREEN  2
#define DARK_BLUE   1
#define DARK_YELLOW 14
#define WHITE       15

#define KEY_UP    0x1
#define KEY_DOWN  0x2
#define KEY_LEFT  0x4
#define KEY_RIGHT 0x8

extern int screen_width;
extern int screen_height;
extern int key_pressed;

extern void draw_pixel(int x, int y, int color);


static fx32 car_pos  = FX(0.0f);
static fx32 distance = FX(0.0f);
static fx32 speed    = FX(0.0f);

static fx32 curvature        = FX(0.0f);
static fx32 track_curvature  = FX(0.0f);
static fx32 player_curvature = FX(0.0f);
static fx32 track_distance   = FX(0.0f);

typedef struct
{
    fx32 curvature;
    fx32 distance;
} track_section_t;

static track_section_t track_sections[] = {{FX(1.0f), FX(200.0f)},
                                           {FX(0.0f), FX(200.0f)},
                                           {FX(1.0f), FX(200.0f)},
                                           {FX(0.0f), FX(400.0f)},
                                           {FX(-1.0f), FX(100.0f)},
                                           {FX(0.0f), FX(200.0f)},
                                           {FX(-1.0f), FX(200.0f)},
                                           {FX(1.0f), FX(200.0f)},
                                           {FX(0.0f), FX(200.0f)},
                                           {FX(0.2f), FX(500.0f)},
                                           {FX(0.0f), FX(200.0f)}};


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

void init()
{
    // find position on track
    for (unsigned int i = 0; i < sizeof(track_sections) / sizeof(track_section_t); ++i)
    {
        track_distance += track_sections[i].distance;
    }
}

void update(fx32 elapsed_time)
{
    if (key_pressed & KEY_UP)
    {
        speed += MUL(FX(2.0f), elapsed_time);
    }
    else
    {
        speed -= MUL(FX(1.0f), elapsed_time);
    }

    if (key_pressed & KEY_LEFT)
        player_curvature -= MUL(FX(0.7f), elapsed_time);

    if (key_pressed & KEY_RIGHT)
        player_curvature += MUL(FX(0.7f), elapsed_time);

    fx32 delta_curvature = player_curvature - track_curvature;
    if (ABS(delta_curvature) >= FX(0.8f))
        speed -= MUL(FX(5.0f), elapsed_time);

    // clamp speed
    if (speed < FX(0.0f))
        speed = FX(0.0f);
    if (speed > FX(1.0f))
        speed = FX(1.0f);

    // move car along track according to car speed
    distance += MUL(MUL(FX(70.0f), speed), elapsed_time);

    // get point on track
    fx32         offset        = 0;
    unsigned int track_section = 0;

    if (distance >= track_distance)
        distance -= track_distance;

    // find position on track
    while (track_section < sizeof(track_sections) / sizeof(track_section_t) && offset < distance)
    {
        offset += track_sections[track_section].distance;
        track_section++;
    }

    fx32 target_curvature = track_section > 0 ? track_sections[track_section - 1].curvature : FX(0.0f);
    fx32 track_curve_diff = MUL(MUL(target_curvature - curvature, elapsed_time), speed);
    curvature += track_curve_diff;

    track_curvature += MUL(MUL(curvature, elapsed_time), speed);

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

            if (xx >= 0 && x < left_grass)
                draw_pixel(x, row, grass_color);
            if (xx >= left_grass && x < left_clip)
                draw_pixel(x, row, clip_color);
            if (xx >= left_clip && x < right_clip)
                draw_pixel(x, row, GRAY);
            if (xx >= right_clip && x < right_grass)
                draw_pixel(x, row, clip_color);
            if (xx >= right_grass && x < FXI(screen_width))
                draw_pixel(x, row, grass_color);
        }
    }

    // draw car
    car_pos = player_curvature - track_curvature;
    int x   = screen_width / 2 + INT(DIV2(MUL(FXI(screen_width), car_pos), FX(2.0f)) - FXI(7));
    int y   = screen_height - 20;

    draw_sprite_line(x, y++, "     x     ", BLACK);
    draw_sprite_line(x, y++, " xx xxx xx ", BLACK);
    draw_sprite_line(x, y++, " xxxxxxxxx ", BLACK);
    draw_sprite_line(x, y++, " xx xxx xx ", BLACK);
    draw_sprite_line(x, y++, "    xxx    ", BLACK);
    draw_sprite_line(x, y++, "xxx xxx xxx", BLACK);
    draw_sprite_line(x, y++, "xxxxxxxxxxx", BLACK);
    draw_sprite_line(x, y++, "xxx xxx xxx", BLACK);
}