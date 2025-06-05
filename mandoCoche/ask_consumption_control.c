#include "ask_consumption_control.h"
#include <string.h>
#include "nRF24L01_TX.h"
#include "tm_stm32_nrf24l01.h"
#include "app_main.h"

#define GET_CONSUMPTION_TIME           1000     //El consumo se pide al coche cada segundo

osThreadId_t id_thread__askConsumptionControl = NULL;

void thread__askConsumptionControl (void *no_argument)
{
    nRF_data_transmitted_t nRF_data; //Mensaje hacia RF para que envie el comando con los datos
    while (1)
    {
        //Manda comando de preguntar consumo al coche
        nRF_data.command = nRF_CMD__ASK_CONSUMPTION;
        if (osMessageQueuePut(id_queue__nRF_TX_Data, &nRF_data, NULL, osWaitForever) != osOK)
        {
            strncpy(detalleError, "MSG QUEUE ERROR        ", sizeof(detalleError) - 1);
            osThreadFlagsSet(id_thread__app_main, FLAG__ERROR);
        }
        //Manda comando para recibir el consumo del coche
        nRF_data.command = nRF_CMD__RECIEVE_CONSUMPTION;
        if (osMessageQueuePut(id_queue__nRF_TX_Data, &nRF_data, NULL, osWaitForever) != osOK)
        {
            strncpy(detalleError, "MSG QUEUE ERROR        ", sizeof(detalleError) - 1);
            osThreadFlagsSet(id_thread__app_main, FLAG__ERROR);
        }
        osDelay(GET_CONSUMPTION_TIME);
    }
}

void Init_askConsumptionControl (void) //Comienza el control de pregunta / respuesta
{
    if (id_thread__askConsumptionControl == NULL)
    {
        id_thread__askConsumptionControl = osThreadNew(thread__askConsumptionControl, NULL, NULL);
        if (id_thread__askConsumptionControl == NULL)
        {
            //Error
            strncpy(detalleError, "THREAD CONSUMPTION ERROR", sizeof(detalleError) - 1);
            osThreadFlagsSet(id_thread__app_main, FLAG__ERROR);
        }
    }
}

void Stop_askConsumptionControl (void)  //Para el control de pregunta / respuesta
{
    if (id_thread__askConsumptionControl != NULL)
    {
        osThreadTerminate(id_thread__askConsumptionControl);
        id_thread__askConsumptionControl = NULL;
    }
}
