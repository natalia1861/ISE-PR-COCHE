#ifndef __LEDS_CONTROL_H
#define __LEDS_CONTROL_H

#include "cmsis_os2.h"

#define GET_MASK_LED(leds)      (1 << leds)

extern uint8_t leds_activate_mask;

void Init_LedsControl(void);

#endif
