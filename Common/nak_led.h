#ifndef __NAK_LED_H
#define __NAK_LED_H

#include "stm32f4xx_hal.h"

#define LED_GREEN           0x00
#define LED_BLUE            0x01
#define LED_RED             0x02

void INITIALIZE_LEDS (void);
void RGB_mbed(void);

void LED_GREEN_ON (void);
void LED_GREEN_OFF(void);
void LED_BLUE_ON (void);
void LED_BLUE_OFF(void);
void LED_RED_ON (void);
void LED_RED_OFF(void);

#endif
