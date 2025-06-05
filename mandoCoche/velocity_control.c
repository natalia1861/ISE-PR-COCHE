#include <stdio.h>
#include <string.h>
#include "velocity_control.h"
#include <stdlib.h>
#include "nRF24L01_TX.h"
#include "tm_stm32_nrf24l01.h"
#include "app_main.h"
#include "adc.h"
//#include "servomotor.h"

#define VELOCITY_REFRESH                    100         //La velocidad / presion / marchas se mira cada 100ms para ver si ha cambiado el valor

osThreadId_t id_thread__velocityControl;
char marcha_S[80]; //Variable para compartir en web

void thread__VelocityControl (void *no_argument)
{
    uint32_t flags;
    nRF_data_transmitted_t nRF_data;  //Mensaje hacia RF para que envie el comando con los datos

    //Variables de control para que si la marcha no cambia, no se envia comando
    marchas_t marcha_prev;
    marchas_t marcha;
    
    //Inicializamos el ADC de control de presión
	Init_ADC1_presion();

    #ifdef TEST_SERVOS
        Init_Servomotors();
    #endif
    
    //Primera iteraccion
    nRF_data.command = nRF_CMD__VELOCITY;   //Se incluye el comando de velocidad
    marcha_prev = getPedal();
    nRF_data.auxiliar_data = marcha_prev;
    if (osMessageQueuePut(id_queue__nRF_TX_Data, &nRF_data, NULL, osWaitForever) != osOK)   //Se añade a la cola de envio de RF
    {
        strncpy(detalleError, "MSG QUEUE ERROR        ", sizeof(detalleError) - 1);
        osThreadFlagsSet(id_thread__app_main, FLAG__ERROR);
    }
	
    while(1)
    {
        #ifdef VELOCITY_TEST //TEST
        velocidad = velocidad + 1;
        if (osMessageQueuePut(id_queue__nRF_TX_Data, &nRF_data, NULL, osWaitForever) != osOK)   //Se aÃ±ade a la cola de envio de RF
        {
            strncpy(detalleError, "MSG QUEUE ERROR        ", sizeof(detalleError) - 1);
            osThreadFlagsSet(id_thread__app_main, FLAG__ERROR);
        }
        #else 
            marcha = getPedal();
            if (marcha != marcha_prev)
            {
                nRF_data.command = nRF_CMD__VELOCITY;
                nRF_data.auxiliar_data = (marcha) ;
                marcha_prev = marcha;

                #ifndef TEST_SERVOS
                //printf("Marcha: %d\n", marcha);
                if (osMessageQueuePut(id_queue__nRF_TX_Data, &nRF_data, NULL, osWaitForever) != osOK)   //Se aÃ±ade a la cola de envio de RF
                {
                    strncpy(detalleError, "MSG QUEUE ERROR        ", sizeof(detalleError) - 1);
                    osThreadFlagsSet(id_thread__app_main, FLAG__ERROR);
                }
                
                //Se actualiza la informacion de Web
                sprintf(marcha_S,"%02d", marcha);
                
                #else   //Prueba Servomotores conectado al mando (para medir frecuencia)
                
                //Se actualiza la informacion de Web
                sprintf(marcha_S,"%02d", marcha);
                
                setMotorSpeed( (speed_marchas_t) marcha);
                #endif
            }
            osDelay(VELOCITY_REFRESH);
        #endif
    }
}

void Init_VelocityCointrol (void)
{
    if ((id_thread__velocityControl = osThreadNew(thread__VelocityControl, NULL, NULL)) == NULL)
    {
        strncpy(detalleError, "THREAD ERROR Velocidad", sizeof(detalleError) - 1);
        osThreadFlagsSet(id_thread__app_main, FLAG__ERROR);
    }
}
