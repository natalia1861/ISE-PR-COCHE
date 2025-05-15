#include "velocity_control.h"
#include <stdlib.h>
#include "cmsis_os2.h"                  // ::CMSIS:RTOS2
#include "nRF24L01_TX.h"
#include "tm_stm32_nrf24l01.h"
#include "app_main.h"

#define VELOCITY_THRESHOLD                  100  //revisarMSP revisarNAK ajustar segun sensbilidad real
void thread__VelocityControl (void)
{
    nRF_data_transmitted_t nRF_data;
    uint16_t velocidad_prev = 0;
    uint16_t velocidad = 0;

    nRF_data.command = nRF_CMD__VELOCITY;   //Se añade el comando de velocidad
    while(1)
    {
        #ifdef VELOCITY_TEST
        velocidad = velocidad + 1;
        #else
        //Implementar funcion de getVelocidad con un THRESHOLD - circuito de presion revisarMSP
        //velocidad = getVelocidad();
        #endif
        if (abs(velocidad_prev - velocidad) >= VELOCITY_THRESHOLD)
        {
            nRF_data.auxiliar_data = velocidad;
            velocidad_prev = velocidad;
            if (osMessageQueuePut(id_queue__nRF_TX_Data, &nRF_data, NULL, osWaitForever) != osOK)   //Se añade a la cola de envio de RF
            {
                osThreadFlagsSet(id_thread__app_main, FLAG__ERROR);
            }
        }
    }
}
