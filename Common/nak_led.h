#ifndef __NAK_LED_H
#define __NAK_LED_H

#include "stm32f4xx_hal.h"

extern void INITIALIZE_LEDS (void);
extern void LED_GREEN_ON (void);
extern void LED_GREEN_OFF(void);
extern void LED_BLUE_ON (void);
extern void LED_BLUE_OFF(void);
extern void LED_RED_ON (void);
extern void LED_RED_OFF(void);

//from LED_NUCLEO_F429ZI
extern int32_t  LED_Initialize   (void);
extern int32_t  LED_Uninitialize (void);
extern int32_t  LED_On           (uint32_t num);
extern int32_t  LED_Off          (uint32_t num);
extern int32_t  LED_SetOut       (uint32_t val);
extern uint32_t LED_GetCount     (void);

#endif
