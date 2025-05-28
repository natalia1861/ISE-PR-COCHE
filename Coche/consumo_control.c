#include "consumo_control.h"
#include "cmsis_os2.h"                  // ::CMSIS:RTOS2
#include "adc.h"

uint16_t consumption = 0;
osThreadId_t id_thread__GetConsumption;

void thread__GetConsumption (void *no_argument)
{
    Init_ADC1_consumo();
    while(1)
    {
        consumption = (uint16_t) (getConsumo() * 1000); //revisarNAK
    }
}

void Init_GetConsumption(void)
{
    if (id_thread__GetConsumption == NULL)
    {
        id_thread__GetConsumption = osThreadNew(thread__GetConsumption, NULL, NULL);
    }
}