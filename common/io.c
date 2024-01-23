#include "io.h"

#include <machine.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>

#define CMD_MODE_SET  0x10
#define CMD_ACK       0xff

#define MODE_PS2      0x80

static bool g_duart_detected = false;
static CharDevice g_duart_device;

bool check_char() {
    if (g_duart_detected)
        return mcCheckDevice(&g_duart_device);
    else
        return mcCheckchar();
}

char read_char() {
    if (g_duart_detected)
        return mcReadDevice(&g_duart_device);
    else
        return mcReadchar();
}

static unsigned char receive_byte(bool is_blocking, bool * is_byte_received)
{
    if (is_blocking)
        while (!check_char());

    if (is_blocking || check_char()) {
        unsigned char c = read_char();
        if (is_byte_received)
            *is_byte_received = true;
        return c;
    }

    if (is_byte_received)
        *is_byte_received = false;
    return 0;
}

void send_byte(unsigned char c) {
    if (g_duart_detected)
        mcSendDevice(c, &g_duart_device);
    else
        mcPrintchar(c);
}

bool find_device(CharDevice *found_device, uint8_t device_type)
{
    bool ret = false;

    uint16_t count = mcGetDeviceCount();
    for (int i = 0; i < count; i++) {
        CharDevice dev;
        if (mcGetDevice(i, &dev)) {
            // if the device has the good type
            if (dev.device_type == device_type) {
                *found_device = dev;
                ret = true;
                break;
            }
        }
    }
    return ret;
}

bool init_io(bool use_port_b_if_available)
{
    // if the device support is available in the firmware, use the DUART
    if (mcCheckDeviceSupport()) {
        // find the DUART device for the given port
        if (find_device(&g_duart_device, use_port_b_if_available ? 0x03 : 0x02)) {
            g_duart_detected = true;
        } else {
            if (use_port_b_if_available) {
                // fallback to port A of the DUART if possible
                if (find_device(&g_duart_device, 0x02))
                    g_duart_detected = true;
            }
        }
    }

    // clear queued events
    bool is_byte_received;
    do
    {
        receive_byte(false, &is_byte_received);
    } while (is_byte_received);

    // enable PS/2 mode
    send_byte(CMD_MODE_SET);
    unsigned char c = receive_byte(true, NULL);
    if (c != CMD_ACK)
        return false;
    send_byte(MODE_PS2);
    c = receive_byte(true, NULL);
    if (c != CMD_ACK)
        return false;
    
    return true;    
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