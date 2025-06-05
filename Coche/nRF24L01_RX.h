#ifndef __nRF24L01_RX
#define __nRF24L01_RX

#include "stm32f4xx_hal.h"

//Flags to id_thread__RF_RX
#define nRF_DATA_READY              0x0001

typedef struct
{
    uint16_t direccion;
    uint16_t velocidad;
} nRF_data_received_coche_t;

void Init_RF_RX(void);
void HAL_GPIO_EXTI_Callback_NRF(uint16_t GPIO_Pin);
#endif
