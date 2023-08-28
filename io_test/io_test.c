#include <io.h>

#include <basicio.h>
#include <stdio.h>

int main(int argc, char ** argv)
{
    (void)argc;
    (void)argv;

    print("\033[H\033[2J");
    println("I/O Test");

    init_io();

    while (1)
    {
        io_event_t io_event;
        if (receive_event(&io_event))
        {
            if (io_event.type == IO_KEYDOWN)
            {
                printf("Key down: scancode=%02x\n", io_event.scancode);
            }
            else if (io_event.type == IO_KEYUP)
            {
                printf("Key up: scancode=%02x\n", io_event.scancode);
            }
            else if (io_event.type == IO_MOUSEMOTION)
            {
                printf("Mouse motion: mstat=%02x, mx=%d, my=%d\n", io_event.mstat, io_event.mx, io_event.my);
            }
        }
    }

    readchar();

    return 0;
}
