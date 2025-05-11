#ifndef __THSERVO_H
#define __THSERVO_H

/* Includes ------------------------------------------------------------------*/
#include "stm32f4xx_hal.h"
#include "cmsis_os2.h"

#define CLOSE_FLAG    0x01
#define OPEN_FLAG     0x02
#define STOP_FLAG			0x04

/* Exported types ------------------------------------------------------------*/
/* Exported constants --------------------------------------------------------*/
/* Exported macro ------------------------------------------------------------*/
/* Exported functions ------------------------------------------------------- */
osThreadId_t Init_ThServo (void);
  /* Exported thread functions,  
  Example: extern void app_main (void *arg); */
#endif /* __MAIN_H */
