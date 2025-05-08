#include "consumption.h"
#include "cmsis_os2.h"
#include <stdlib.h>

//revisarNAK revisar consumo
/* Hilo que se va a encargar de obtener las medidas de consumo y enviarlas a la memoria flash y a Web*/
void thread__GetComsumtion (void *no_argument)
{
    uint16_t consumo_prev;
    uint16_t consumo;
    //revisarMSP
    //Init_comsumo() //descomentar cuando se inicie el consumo
    for (;;)
    {
        //consumo = getConsumo()
        //FLASH__WriteConsumption(consumo);
        //mandar a WEB
        //sendToWeb()
    }
}

void Init_thread_GetConsumption (void)
{
    //iniciar el hilo de consumo
    osThreadNew(thread__GetComsumtion, NULL, NULL);
}
