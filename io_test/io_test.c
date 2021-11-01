#include <basicio.h>
#include <gpio.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdnoreturn.h>

const int rosco_sck  = GPIO2;
const int rosco_mosi = GPIO3;
const int rosco_miso = GPIO4;

const int sck_period = 100;

static void init_gpios()
{
    pinMode(GPIO1, OUTPUT);        // CS
    pinMode(GPIO2, OUTPUT);        // SCK
    pinMode(GPIO3, OUTPUT);        // MOSI
    pinMode(GPIO4, INPUT);         // MISO
}

static void set_gpios(bool g2, bool g3)
{
    digitalWrite(GPIO2, g2);
    digitalWrite(GPIO3, g3);
}

bool receive_bit()
{
    int b;

    digitalWrite(rosco_sck, true);
    delay(sck_period);
    b = digitalRead(rosco_miso);
    digitalWrite(rosco_sck, false);
    delay(sck_period);

    return b != 0;
}

unsigned char receive_byte(bool is_blocking, bool * is_byte_received)
{
    unsigned char c = 0;

    if (is_byte_received)
        *is_byte_received = false;

    // wait until the start bit is received
    while (!receive_bit())
    {
        if (!is_blocking)
            return 0;
    }

    for (int i = 0; i < 8; ++i)
    {
        c <<= 1;
        c |= (receive_bit() ? 1 : 0);
    }

    if (is_byte_received)
        *is_byte_received = true;
    return c;
}

int main(int argc, char ** argv)
{
    (void)argc;
    (void)argv;

    print("\033[H\033[2J");
    println("I/O Test");
    println("(press a key to exit)");
    init_gpios();

    while (!checkchar())
    {
        bool          is_byte_received;
        unsigned char c;

        c = receive_byte(false, &is_byte_received);

        if (is_byte_received)
        {
            if (c == 'K')
            {
                unsigned char scancode = receive_byte(true, NULL);
                printf("Keyboard: %02x\n", scancode);
            }
            else if (c == 'M')
            {
                unsigned char mstat = receive_byte(true, NULL);
                char          mx    = (char)receive_byte(true, NULL);
                char          my    = (char)receive_byte(true, NULL);
                printf("Mouse: %02x %02x %02x\n", mstat, mx, my);
            }
        }
    }

    readchar();

    return 0;
}
