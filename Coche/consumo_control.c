#include "consumo_control.h"
#include "cmsis_os2.h"                  // ::CMSIS:RTOS2
#include "adc.h"

//El valor de consumo se actualiza en el coche cada CONSUMPTION_UPDATE_TIME, pero se envia a RF segun cada cuanto tiempo pregunte el mando (definido en ask_consumption_control)
#define CONSUMPTION_UPDATE_TIME                 200 //ms

uint16_t consumption = 0;                       //Variable global de consumo
osThreadId_t id_thread__GetConsumption;

//Hilo que va a ir obteniendo el consumo
void thread__GetConsumption (void *no_argument)
{
    Init_ADC1_consumo();    //Inicializacion de ADC 1 para consumo (Utilizamos el ADC1 canal 10 pin PC3)
    while(1)
    {
        consumption = (uint16_t) (getConsumo() * 1000); //Se pasa el consumo como un uin16_t con 3 decimales
        osDelay(CONSUMPTION_UPDATE_TIME);
    }
}

//Hilo que inicializa el consumo
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
