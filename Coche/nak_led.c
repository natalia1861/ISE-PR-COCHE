#include "stm32f4xx_hal.h"
#include "nak_led.h"

#define LED_GREEN           0x01
#define LED_BLUE            0x02
#define LED_RED             0x04

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
