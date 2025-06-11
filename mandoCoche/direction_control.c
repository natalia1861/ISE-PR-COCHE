#include "direction_control.h"
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "nRF24L01_TX.h"
#include "tm_stm32_nrf24l01.h"
#include "app_main.h"
#include "sensor_AS5600.h"
//#include "servomotor.h"

#define DIRECTION_THRESHOLD                         0.1  //Sensibilidad para variar el valor guardado //revisar, segura que no es 1?¿
#define DIRECTION_REFRESH                           100

osThreadId_t id_thread__DirectionControl = NULL;
char direccion_S[80];       //Variable de direccion para Web revisar NAK falta comentar

void thread__direction_control(void *no_argument)
{
    float direction = 0;
    float direction_prev = 0;

    nRF_data_transmitted_t nRF_data; //Mensaje hacia RF para que envie el comando con los datos

    nRF_data.command = nRF_CMD__DIRECTION;
    
    as5600_init();  //Inicializamos el sensor angular AS5600
    while(1)
    {
        #ifdef DIRECTION_TEST //TEST
        direction = direction + 1;
        nRF_data.auxiliar_data = direction;
        if (osMessageQueuePut(id_queue__nRF_TX_Data, &nRF_data, NULL, osWaitForever) != osOK)   //Se anade a la cola de envio de RF
        {
            strncpy(detalleError, "MSG QUEUE ERROR DIR", sizeof(detalleError) - 1);
            osThreadFlagsSet(id_thread__app_main, FLAG__ERROR);
        }
        #else   //Aplicacion
        if (!(as5600_readout(&direction) == AS5600_OK)) //revisar funcion para enviar errores, sobre todo con lectura de iman
        {
            //Error
            strncpy(detalleError, "AS5600 Error       ", sizeof(detalleError) - 1);
            osThreadFlagsSet(id_thread__app_main, FLAG__ERROR);
        }
            
        if (fabsf(direction - direction_prev) >= DIRECTION_THRESHOLD) //Si la direccion varia lo suficiente
        {
            direction_prev = direction;

            #ifndef TEST_SERVOS
            //Se pasa a uint16_t con dos decimales (Representa valores 0.00 a 655.35)
            nRF_data.auxiliar_data = (uint16_t)(direction * 100);
            direction_prev = direction;
            if (osMessageQueuePut(id_queue__nRF_TX_Data, &nRF_data, NULL, osWaitForever) != osOK)   //Se incluye a la cola de envio de RF
            {
                strncpy(detalleError, "MSG QUEUE ERROR DIR", sizeof(detalleError) - 1);
                osThreadFlagsSet(id_thread__app_main, FLAG__ERROR);
            }
            
            #else 
            //setServoAngle(direction);
            #endif
            //Se pasa a Web
            sprintf(direccion_S, "%.2f", direction);
        }
        #endif
        osDelay(DIRECTION_REFRESH);
    }
}

void Init_DirectionControl (void)       //Se inicia el control de deteccion y envio de direccion (marchas)
{
    if (id_thread__DirectionControl == NULL)
    {
        id_thread__DirectionControl = osThreadNew(thread__direction_control, NULL, NULL);
        if (id_thread__DirectionControl == NULL)
        {
            //ERROR
            strncpy(detalleError, "THREAD AS5600 ERROR", sizeof(detalleError) - 1);
            osThreadFlagsSet(id_thread__app_main, FLAG__ERROR);
        }
    }
}
