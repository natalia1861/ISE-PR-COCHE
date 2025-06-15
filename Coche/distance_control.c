#include "distance_control.h"
#include "cmsis_os2.h"                  // ::CMSIS:RTOS2
#include "VL53L0X.h"
#include "i2c.h"
#include "errors.h"

//El valor de distancia  se actualiza en el coche cada DISTANCE_UPDATE_TIME, pero se envia a RF segun cada cuanto tiempo pregunte el mando (mirar en ask_distance_control)
#define DISTANCE_UPDATE_TIME                 100 //ms

#define myoffset        10                  //Offset aplicado al sensor distancia para calibracion
#define myscale         1.05                //Factor de escala aplicado (ajusta la distancia medida si hay una relacion lineal diferente a 1:1)

uint16_t distancia = 0; //Variable global de distancia
VL53L0X sensor1;

extern I2C_HandleTypeDef hi2c2; //I2C2

osThreadId_t id_thread__DistanceControl = NULL;

void thread__distance_control (void *no_argument)
{
    startContinuous(&sensor1,0); //Second parameter: x ms wait -> realiza medidas continuamente
	osDelay(2000);  //Tiempo obligatorio de espera para configuracion
    
    for(;;)
    {
        //Obtener distancia del sensor distancia y actualizar variable global
        #ifdef DISTANCIA_TEST
        distancia = (distancia == 5000) ? 0 : distancia + 50;
        #else
        distancia = (uint16_t) ((readRangeContinuousMillimeters(&sensor1)*myscale)-myoffset); //Lee la distancia en mm
        #endif
        osDelay(DISTANCE_UPDATE_TIME);
    }
}

//Inicializacion del control de distancia
void Init_DistanceControl (void)
{
    if (id_thread__DistanceControl == NULL)
    {
       id_thread__DistanceControl = osThreadNew(thread__distance_control, NULL, NULL);
       if (id_thread__DistanceControl == NULL)
       {
            //Error revisar como mandar a RF
       }
    }
}

//Para el sensor para ahorrar energia
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
    MX_I2C2_Init();             //Inicializa el perif�rico I2C2 del STM32          
    HAL_I2C_MspInit(&hi2c2);
    InitVL53(&sensor1);
}
