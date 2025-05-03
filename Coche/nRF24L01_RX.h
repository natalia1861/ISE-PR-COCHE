#ifndef __nRF24L01_RX
#define __nRF24L01_RX

#include "stm32f4xx_hal.h"

void Init_RF_RX(void);
void HAL_GPIO_EXTI_Callback_NRF(uint16_t GPIO_Pin);
#endif
