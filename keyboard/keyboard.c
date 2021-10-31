#include <basicio.h>
#include <gpio.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdnoreturn.h>

const int rosco_sck  = GPIO2;
const int rosco_mosi = GPIO3;
const int rosco_miso = GPIO4;

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

const int sck_period = 100;
bool      receive_bit()
{
    int b;

    digitalWrite(rosco_sck, true);
    delay(sck_period / 2);
    b = digitalRead(rosco_miso);
    delay(sck_period / 2);
    digitalWrite(rosco_sck, false);
    delay(sck_period);

    return b != 0;
}

char receive_char()
{
    char c = 0;

    if (receive_bit())
    {
        for (int i = 0; i < 8; ++i)
        {
            c <<= 1;
            c |= (receive_bit() ? 1 : 0);
        }
    }

    return c;
}

int main(int argc, char ** argv)
{
    (void)argc;
    (void)argv;

    print("\033[H\033[2J");
    println("Keyboard Test");
    println("(press a key to exit)");
    init_gpios();

    while (!checkchar())
    {
        char c = receive_char();
        if (c)
        {
            printf("%c", c);
        }
    }

    readchar();

    return 0;
}
