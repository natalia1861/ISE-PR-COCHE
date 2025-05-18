#include "ask_distance_control.h"
#include <string.h>
#include "nRF24L01_TX.h"
#include "tm_stm32_nrf24l01.h"
#include "app_main.h"

#define GET_DISTANCE_TIME           200

osThreadId_t id_thread__askDistanceControl = NULL;
char distancia_S[80];

void thread__askDistanceControl (void *no_argument)
{
    nRF_data_transmitted_t nRF_data;
    while (1)
    {
        nRF_data.command = nRF_CMD__ASK_DISTANCE;
        if (osMessageQueuePut(id_queue__nRF_TX_Data, &nRF_data, NULL, 1000) != osOK)
        {
            strncpy(detalleError, "MSG QUEUE ERROR        ", sizeof(detalleError) - 1);
            osThreadFlagsSet(id_thread__app_main, FLAG__ERROR);
        }
        nRF_data.command = nRF_CMD__RECEIVE_DISTANCE;
        if (osMessageQueuePut(id_queue__nRF_TX_Data, &nRF_data, NULL, 1000) != osOK)
        {
            strncpy(detalleError, "MSG QUEUE ERROR        ", sizeof(detalleError) - 1);
            osThreadFlagsSet(id_thread__app_main, FLAG__ERROR);
        }
        osDelay(GET_DISTANCE_TIME);
    }
}

void Init_askDistanceControl (void)
{
    if (id_thread__askDistanceControl == NULL)
        id_thread__askDistanceControl = osThreadNew(thread__askDistanceControl, NULL, NULL);
    if (id_thread__askDistanceControl == NULL)
    {
        strncpy(detalleError, "THREAD DISTANCE ERROR", sizeof(detalleError) - 1);
        osThreadFlagsSet(id_thread__app_main, FLAG__ERROR);
    }
}

void Stop_askDistanceControl (void)
{
    if (id_thread__askDistanceControl != NULL)
    {
        osThreadTerminate(id_thread__askDistanceControl);
        id_thread__askDistanceControl = NULL;
    }
}
