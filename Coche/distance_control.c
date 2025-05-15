#include "distance_control.h"
#include "cmsis_os2.h"                  // ::CMSIS:RTOS2

uint16_t distancia = 0;

void thread__distance_control (void *no_argument)
{
    
    for(;;)
    {
        //Obtener distancia del sensor distancia y actualizar variable global
        #ifdef DISTANCIA_TEST
        distancia = (distancia == 5000) ? 0 : distancia + 50;
        #else
        //distancia = getDistancia(); //revisarMSP
        #endif
        osDelay(100);
    }
}

void Init_DistanceControl (void)
{
    osThreadNew(thread__distance_control, NULL, NULL);
}
