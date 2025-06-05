#ifndef __nRF24L01_TX
#define __nRF24L01_TX

#include "stm32f4xx_hal.h"
#include "cmsis_os2.h"

//Flags to thread__GetData_RF_TX
#define nRF_DATA_READY              0x0001

typedef struct
{
    uint8_t command;
    uint16_t auxiliar_data;
} nRF_data_transmitted_t;    //Mensaje hacia RF para que envie el comando con los datos

typedef struct
{
    uint16_t distancia;
    uint16_t consumo;
} nRF_data_received_mando_t;

extern osMessageQueueId_t id_queue__nRF_TX_Data;
extern nRF_data_received_mando_t nRF_data_received_mando;

void Init_RF_TX(void);
void HAL_GPIO_EXTI_Callback_NRF(uint16_t GPIO_Pin);

#endif
