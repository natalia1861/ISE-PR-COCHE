#ifndef __THTEMP_H
#define __THTEMP_H

#include "cmsis_os2.h"
#include "Driver_I2C.h"
#include "RTE_Device.h"
#include "stm32f4xx_hal.h"

//to principal.c
#define TEMP_FLAG 												0x02

//local
#define SIZE_MSGQUEUE_TEMP 3
#define TRANSFER_TEMP_COMPLETE 0x01

typedef struct{
  float temp;
} MSGQUEUE_TEMP_t;

void initModTemp(void);
osMessageQueueId_t getTempQueueID(void);
void leer_temp (void);
void thread__temp (void *no_argument);

extern osThreadId_t tid_Thread_sensor;


#endif
