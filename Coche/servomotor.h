#ifndef __SERVOMOTORS_H
#define __SERVOMOTORS_H
#include <stdbool.h>
#include "stm32f4xx_hal.h"

typedef enum
{
    MARCHA_0 = 0,
    MARCHA_1 = 1,
    MARCHA_2
} speed_marchas_t;

void Init_Servomotors (void);
void setMotorSpeed(speed_marchas_t speed);

#endif
