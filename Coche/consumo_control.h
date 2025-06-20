#ifndef __CONSUMO_CONTROL_H
#define __CONSUMO_CONTROL_H

#include "cmsis_os2.h"                  // ::CMSIS:RTOS2

extern uint16_t consumption;

void Init_GetConsumption(void);
void DeInit_GetConsumption(void);

#endif


