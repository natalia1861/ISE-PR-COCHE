#include "distance_control.h"
#include "cmsis_os2.h"                  // ::CMSIS:RTOS2

uint16_t distancia = 0;

void thread__distance_control (void *no_argument)
{
    
    for(;;)
    {
        //Obtener distancia del sensor distancia y actualizar variable global
        #ifdef DISTANCIA_TEST
        distancia = distancia + 1;
        #else
        //distancia = getDistancia(); //revisarMSP
        #endif
        osDelay(100);
    }
}
