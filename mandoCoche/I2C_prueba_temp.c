#include "I2C_prueba_temp.h"

osTimerId_t id_timer__measureTemperature = NULL;
osThreadId_t id_thread__measureTemperature = NULL;

float temp_aux;
uint64_t temperatura;


static void timer__mesureTemperature_callback (void *no_argument) {
    osThreadFlagsSet(id_thread__measureTemperature, FLAG_TEMP);
}

void thread__measureTemperature (void *argument) {
    uint32_t value;
    float temperatura_real;
    uint8_t buff[2];
    I2C_Init_All();
    id_timer__measureTemperature = osTimerNew (timer__mesureTemperature_callback, osTimerPeriodic, NULL, NULL);
    osTimerStart(id_timer__measureTemperature, 100);
    for(;;) {
    osThreadFlagsWait(FLAG_TEMP, osFlagsWaitAny, osWaitForever);
    I2C_ReadRegisters(I2C_LINE_1, m_ADDR, REG_TEMP, buff, 2);
    temperatura = (buff[0] << 8) | buff[1];
    //Read the 11-bit raw temperature value
    value = temperatura >> 5;
 
    //Sign extend negative numbers
    if (value & (1 << 10))
        value |= 0xFC00;// this operation sets the upper 10 bits of value to 1, while leaving the lower bits unchanged.
 
    //Return the temperature in °C
    temp_aux = value * 0.125;//So, if the 10th bit of value is set, the upper 10 bits of value will be set to 1 by performing a bitwise OR operation with 0xFC00. If the 10th bit is not set, nothing happens.
    }
}

void Init_temp_sensor(void) {
    id_thread__measureTemperature = osThreadNew (thread__measureTemperature, NULL, NULL);
}


