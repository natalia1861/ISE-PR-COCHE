#include "distance_control.h"
#include "cmsis_os2.h"                  // ::CMSIS:RTOS2

uint16_t distancia = 0;

osThreadId_t id_thread__DistanceControl = NULL;

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
    if (id_thread__DistanceControl == NULL)
    {
       id_thread__DistanceControl = osThreadNew(thread__distance_control, NULL, NULL);
    }
}

void Stop_DistanceControl (void)
{
    if (id_thread__DistanceControl != NULL)
    {
        osThreadTerminate(id_thread__DistanceControl);
        id_thread__DistanceControl = NULL;
    }
}
