#include "direction_control.h"
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "nRF24L01_TX.h"
#include "tm_stm32_nrf24l01.h"
#include "app_main.h"
#include "sensor_AS5600.h"
#include "servomotor.h"

#define DIRECTION_THRESHOLD                         0.1  //revisarPAR revisarNAK ajustar segun sensibilidad real 
#define DIRECTION_REFRESH                           100

osThreadId_t id_thread__DirectionControl = NULL;
char direccion_S[80];

void thread__direction_control(void *no_argument)
{
    float direction = 0;
    float direction_prev = 0;

    nRF_data_transmitted_t nRF_data;

    nRF_data.command = nRF_CMD__DIRECTION;
    
    as5600_init();
    while(1)
    {
        //Implementar funcion de getDirecion con un THRESHOLD
        #ifdef DIRECTION_TEST
        direction = direction + 1;
        nRF_data.auxiliar_data = direction;
        if (osMessageQueuePut(id_queue__nRF_TX_Data, &nRF_data, NULL, osWaitForever) != osOK)   //Se anade a la cola de envio de RF
        {
            strncpy(detalleError, "MSG QUEUE ERROR        ", sizeof(detalleError) - 1);
            osThreadFlagsSet(id_thread__app_main, FLAG__ERROR);
        }
        #else
        if (!(as5600_readout(&direction) == AS5600_OK))
        {
            //Error
            strncpy(detalleError, "AS5600 Error       ", sizeof(detalleError) - 1);
            osThreadFlagsSet(id_thread__app_main, FLAG__ERROR);
        }
            
        if (fabsf(direction - direction_prev) >= DIRECTION_THRESHOLD)
        {
            direction_prev = direction;

            #ifndef TEST_SERVOS
            //Se pasa a uint16_t con dos decimales (Representa valores 0.00 a 655.35)
            nRF_data.auxiliar_data = (uint16_t)(direction * 100);
            direction_prev = direction;
            if (osMessageQueuePut(id_queue__nRF_TX_Data, &nRF_data, NULL, osWaitForever) != osOK)   //Se añade a la cola de envio de RF
            {
                strncpy(detalleError, "MSG QUEUE ERROR        ", sizeof(detalleError) - 1);
                osThreadFlagsSet(id_thread__app_main, FLAG__ERROR);
            }
            
            #else 
            setServoAngle(direction);
            #endif
            //Se pasa a Web
            sprintf(direccion_S, "%.2f", direction);
        }
        #endif
        osDelay(DIRECTION_REFRESH);
    }
}

void Init_DirectionControl (void)
{
    if (id_thread__DirectionControl == NULL)
    {
        id_thread__DirectionControl = osThreadNew(thread__direction_control, NULL, NULL);
    }
    else
    {
        //Error
        strncpy(detalleError, "AS5600 Error thread", sizeof(detalleError) - 1);
        osThreadFlagsSet(id_thread__app_main, FLAG__ERROR);
    }
}
