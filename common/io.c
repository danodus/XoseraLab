#include "io.h"

#include <basicio.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>

static unsigned char receive_byte(bool is_blocking, bool * is_byte_received)
{
    if (is_blocking)
        while (!checkchar());

    if (is_blocking || checkchar()) {
        unsigned char c = readchar();
        if (is_byte_received)
            *is_byte_received = true;
        return c;
    }

    if (is_byte_received)
        *is_byte_received = false;
    return 0;
}

void init_io()
{
    // clear queued events
    bool is_byte_received;
    do
    {
        receive_byte(false, &is_byte_received);
    } while (is_byte_received);
}

bool receive_event(io_event_t * io_event)
{
    bool          is_byte_received;
    static bool   is_waiting_key_up_scancode = false;
    static bool   is_ext_key_scancode        = false;
    unsigned char c;

    bool is_event_complete = false;

    c = receive_byte(false, &is_byte_received);
    if (is_byte_received)
    {
        if (c == 'K')
        {
            io_event->scancode = receive_byte(true, NULL);

            if (io_event->scancode == 0xe0)
            {
                // extended scancode
                is_ext_key_scancode = true;
            }
            else
            {
                if (is_waiting_key_up_scancode)
                {
                    io_event->type             = IO_KEYUP;
                    is_waiting_key_up_scancode = false;
                    if (is_ext_key_scancode)
                    {
                        io_event->scancode |= 0xe000;
                        is_ext_key_scancode = false;
                    }

                    is_event_complete = true;
                }
                else
                {
                    if (io_event->scancode == 0xf0)
                    {
                        is_waiting_key_up_scancode = true;
                    }
                    else
                    {
                        io_event->type = IO_KEYDOWN;
                        if (is_ext_key_scancode)
                        {
                            io_event->scancode |= 0xe000;
                            is_ext_key_scancode = false;
                        }
                        is_event_complete = true;
                    }
                }
            }
        }
        else if (c == 'M')
        {
            io_event->type    = IO_MOUSEMOTION;
            io_event->mstat   = receive_byte(true, NULL);
            io_event->mx      = (char)receive_byte(true, NULL);
            io_event->my      = (char)receive_byte(true, NULL);
            is_event_complete = true;
        }
        return is_event_complete;
    }

    return false;
}