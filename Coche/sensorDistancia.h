#ifndef SENSORDISTANCIA_H
#define SENSORDISTANCIA_H

/* Includes ------------------------------------------------------------------*/
#include "stm32f4xx_hal.h"
#include "cmsis_os2.h"
#include "VL53L0X.h"

#include "cmsis_os2.h"
#define PLAZAS_MAX 10
#define DIST_MAX  430 //mm
#define DIST_MED2  310 //mm
#define DIST_MED1  170 //mm
#define DIST_MIN  80 //mm
// Declaración de las funciones de inicialización y threads
void control_aforo(int d);

#endif 
