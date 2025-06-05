#include "consumo_control.h"
#include "cmsis_os2.h"                  // ::CMSIS:RTOS2
#include "adc.h"

//Consumo se actualiza en coche cada CONSUMPTION_UPDATE_TIME, pero se envia a RF segun cada cuanto tiempo pregunte el mando
#define CONSUMPTION_UPDATE_TIME                 200 //ms

uint16_t consumption = 0;
osThreadId_t id_thread__GetConsumption;

//Hilo que va a ir obteniendo el consumo
void thread__GetConsumption (void *no_argument)
{
    Init_ADC1_consumo();
    while(1)
    {
        consumption = (uint16_t) (getConsumo() * 1000);
        osDelay(CONSUMPTION_UPDATE_TIME);
    }
}

void Init_GetConsumption(void)
{
    if (id_thread__GetConsumption == NULL)
    {
        id_thread__GetConsumption = osThreadNew(thread__GetConsumption, NULL, NULL);
        if (id_thread__GetConsumption == NULL)
        {
            //Error revisar como mandar a RF
        }
    }
}