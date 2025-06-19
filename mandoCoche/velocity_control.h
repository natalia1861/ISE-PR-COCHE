#ifndef __VELOCITY_CONTROL_H
#define __VELOCITY_CONTROL_H

#include "cmsis_os2.h"                  // ::CMSIS:RTOS2

#define FLAG__GET_PRESSURE              0x0001

extern osThreadId_t id_thread__velocityControl;

void Init_VelocityControl (void);
void DeInit_VelocityControl (void);

#endif
