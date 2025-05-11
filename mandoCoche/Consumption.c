#include "consumption.h"
#include "cmsis_os2.h"
#include <stdlib.h>

//revisarNAK revisar consumo
/* Hilo que se va a encargar de obtener las medidas de consumo y enviarlas a la memoria flash y a Web*/
void thread__GetComsumtion (void *no_argument)
{
    uint16_t consumo_prev = 0;
    uint16_t consumo = 0;
    //revisarMSP
    //Init_comsumo() //descomentar cuando se inicie el consumo
    for (;;)
    {
        #ifdef CONSUMO_TEST
        consumo = consumo + 1;
        #else
        //consumo = getConsumo()
        //FLASH__WriteConsumption(consumo);
        //mandar a WEB
        //sendToWeb()
        #endif
    }
}

void Init_thread_GetConsumption (void)
{
    //iniciar el hilo de consumo
    osThreadNew(thread__GetComsumtion, NULL, NULL);
}
