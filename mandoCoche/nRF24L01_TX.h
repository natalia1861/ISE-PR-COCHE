#ifndef __nRF24L01_TX
#define __nRF24L01_TX

#include "stm32f4xx_hal.h"
#include "cmsis_os2.h"

typedef struct
{
    uint8_t command;
    uint16_t auxiliar_data;
} nRF_data_t;

extern osMessageQueueId_t id_queue__nRF_TX_Data;

void Init_RF_TX(void);
void HAL_GPIO_EXTI_Callback_NRF(uint16_t GPIO_Pin);

#endif
