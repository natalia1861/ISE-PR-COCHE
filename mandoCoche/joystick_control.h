#ifndef __JOYSTICK_H
#define __JOYSTICK_H

#include "cmsis_os2.h"

#define FLAG_FIN_REB        0x0001
#define FLAG_PULSACION      0x0002
#define FLAG_PL             0x0004

extern osThreadId_t id_thread__joystick_control;
void Init_JoystickControl (void);

#endif
