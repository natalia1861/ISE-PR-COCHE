#include "ask_distance_control.h"
#include <string.h>
#include "nRF24L01_TX.h"
#include "tm_stm32_nrf24l01.h"
#include "app_main.h"
#include "errors.h"

#define GET_DISTANCE_TIME           200     //La distancia se pregunta al coche cada 200ms

osThreadId_t id_thread__askDistanceControl = NULL; 

void thread__askDistanceControl (void *no_argument)
{
    nRF_data_transmitted_t nRF_data_transmitted; //Mensaje hacia RF para que envie el comando con los datos
    while (1)
    {
        //Manda comando de preguntar consumo al coche
        nRF_data_transmitted.command = nRF_CMD__ASK_DISTANCE;
        if (osMessageQueuePut(id_queue__nRF_TX_Data, &nRF_data_transmitted, NULL, 1000) != osOK)
        {
            push_error(MODULE__ASK_DISTANCE, ERR_CODE__QUEUE, 0);
        }
        //Manda comando para recibir el consumo del coche
        nRF_data_transmitted.command = nRF_CMD__RECEIVE_DISTANCE;
        if (osMessageQueuePut(id_queue__nRF_TX_Data, &nRF_data_transmitted, NULL, 1000) != osOK)
        {
            push_error(MODULE__ASK_DISTANCE, ERR_CODE__QUEUE, 1);
        }
        osDelay(GET_DISTANCE_TIME);
    }
}

void Init_askDistanceControl (void) //Comienza el control de pregunta / respuesta
{
    if (id_thread__askDistanceControl == NULL)
        id_thread__askDistanceControl = osThreadNew(thread__askDistanceControl, NULL, NULL);
    
    if (id_thread__askDistanceControl == NULL)
        push_error(MODULE__ASK_DISTANCE, ERR_CODE__THREAD, 0);
}

void Stop_askDistanceControl (void) //Para el control de pregunta / respuesta
{
    if (id_thread__askDistanceControl != NULL)
    {
        osThreadTerminate(id_thread__askDistanceControl);
        id_thread__askDistanceControl = NULL;
    }
}
