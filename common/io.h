#ifndef IO_H
#define IO_H

#include <stdbool.h>

#define IO_SCANCODE_ESC 0x0076

#define IO_SCANCODE_A 0x001C
#define IO_SCANCODE_D 0x0023
#define IO_SCANCODE_S 0x001B
#define IO_SCANCODE_W 0x001D

#define IO_SCANCODE_UP    0xE075
#define IO_SCANCODE_LEFT  0xE06B
#define IO_SCANCODE_DOWN  0xE072
#define IO_SCANCODE_RIGHT 0xE074


#define IO_KEYDOWN     0
#define IO_KEYUP       1
#define IO_MOUSEMOTION 2

typedef struct
{
    int type;

    // Keyboard
    unsigned int scancode;

    // Mouse
    unsigned char mstat;
    char          mx;
    char          my;
} io_event_t;


void init_io(bool use_port_b_if_available);
bool receive_event(io_event_t * io_event);

#endif