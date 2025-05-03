#include "nak_led.h"
#include "Board_LED.h"

#define LED_GREEN           0x00
#define LED_BLUE            0x01
#define LED_RED             0x02

void LED_GREEN_ON (void)
{
    LED_On(LED_GREEN);
}

void LED_GREEN_OFF(void)
{
    LED_Off(LED_GREEN);
}

void LED_BLUE_ON (void)
{
    LED_On(LED_BLUE);
}

void LED_BLUE_OFF(void)
{
    LED_Off(LED_BLUE);
}

void LED_RED_ON (void)
{
    LED_On(LED_RED);
}

void LED_RED_OFF(void)
{
    LED_Off(LED_RED);
}

void INITIALIZE_LEDS (void)
{
    LED_Initialize();
}
