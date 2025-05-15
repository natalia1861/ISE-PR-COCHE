#include "ask_consumption_control.h"
#include "nRF24L01_TX.h"
#include "tm_stm32_nrf24l01.h"
#include "app_main.h"

#define GET_CONSUMPTION_TIME           1000

osThreadId_t id_thread__askConsumptionControl = NULL;

void thread__askConsumptionControl (void *no_argument)
{
    nRF_data_t nRF_data;
    while (1)
    {
        nRF_data.command = nRF_CMD__ASK_DISTANCE;
        if (osMessageQueuePut(id_queue__nRF_TX_Data, &nRF_data, NULL, osWaitForever) != osOK)
        {
            osThreadFlagsSet(id_thread__app_main, FLAG__ERROR);
        }
        osDelay(GET_CONSUMPTION_TIME);
    }
}

void Init_askConsumptionControl (void)
{
    if (id_thread__askConsumptionControl == NULL)
        id_thread__askConsumptionControl = osThreadNew(thread__askConsumptionControl, NULL, NULL);
}

void Stop_askConsumptionControl (void)
{
    if (id_thread__askConsumptionControl != NULL)
    {
        osThreadTerminate(id_thread__askConsumptionControl);
        id_thread__askConsumptionControl = NULL;
    }
}
