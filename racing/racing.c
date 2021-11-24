// Ref.: https://www.youtube.com/watch?v=KkMZI5Jbf18

#include "../common/fx.h"

#include <math.h>

#define KEY_UP    0x1
#define KEY_DOWN  0x2
#define KEY_LEFT  0x4
#define KEY_RIGHT 0x8

extern int screen_width;
extern int screen_height;
extern int key_pressed;

extern void draw_pixel(int x, int y, int color);


fx32 car_pos         = FX(0.0f);
fx32 distance        = FX(0.0f);
fx32 track_curvature = FX(0.0f);
fx32 curvature       = FX(0.0f);

static fx32 speed = FX(0.0f);

static fx32 player_curvature = FX(0.0f);
static fx32 track_distance   = FX(0.0f);

typedef struct
{
    fx32 curvature;
    fx32 distance;
} track_section_t;

static track_section_t track_sections[] = {{FX(0.0f), FX(10.0f)},
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

    // fx32 delta_curvature = player_curvature - track_curvature;
    // if (ABS(delta_curvature) >= FX(0.8f))
    //    speed -= MUL(FX(5.0f), elapsed_time);

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
    car_pos = player_curvature - track_curvature;
}