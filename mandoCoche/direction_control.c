#include "direction_control.h"
#include <stdlib.h>
#include "nRF24L01_TX.h"
#include "cmsis_os2.h"                  // ::CMSIS:RTOS2
#include "tm_stm32_nrf24l01.h"
#include "app_main.h"

#define DIRECTION_THRESHOLD                         50  //revisarPAR revisarNAK ajustar segun sensibilidad real 

void thread__direction_control(void *no_argument)
{
    nRF_data_t nRF_data;
    uint16_t direction_prev = 0;
    uint16_t direction = 0;

    nRF_data.command = nRF_CMD__DIRECTION;
    while(1)
    {
        //Implementar funcion de getDirecion con un THRESHOLD
        #ifdef DIRECTION_TEST
        direction = direction + 1;
        #else
        //revisarPAR
        #endif
        if (abs(direction_prev - direction) > DIRECTION_THRESHOLD)
        {
            nRF_data.auxiliar_data = direction;
            direction_prev = direction;
            if (osMessageQueuePut(id_queue__nRF_TX_Data, &nRF_data, NULL, osWaitForever) != osOK)   //Se añade a la cola de envio de RF
            {
                osThreadFlagsSet(id_thread__app_main, FLAG__ERROR);
            }
        }
    }
}

void Init_DirectionControl (void)
{
    osThreadNew(thread__direction_control, NULL, NULL);
}
