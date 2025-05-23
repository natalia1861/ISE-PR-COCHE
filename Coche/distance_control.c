#include "distance_control.h"
#include "cmsis_os2.h"                  // ::CMSIS:RTOS2
#include "VL53L0X.h"
#include "i2c.h"

#define myoffset        10
#define myscale         1.05

uint16_t distancia = 0;
VL53L0X sensor1;

//from I2C.c
extern I2C_HandleTypeDef hi2c2;

osThreadId_t id_thread__DistanceControl = NULL;

void thread__distance_control (void *no_argument)
{
    startContinuous(&sensor1,0); //Second parameter: x ms wait
	osDelay(2000);
    
    for(;;)
    {
        //Obtener distancia del sensor distancia y actualizar variable global
        #ifdef DISTANCIA_TEST
        distancia = (distancia == 5000) ? 0 : distancia + 50;
        #else
        distancia = (uint16_t)(readRangeContinuousMillimeters(&sensor1)*myscale)-myoffset;
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
    stopContinuous(&sensor1);
    if (id_thread__DistanceControl != NULL)
    {
        osThreadTerminate(id_thread__DistanceControl);
        id_thread__DistanceControl = NULL;
    }
}

void Init_SensorDistancia(void)
{
    MX_I2C2_Init();
    HAL_I2C_MspInit(&hi2c2);
    InitVL53(&sensor1);
}
