#include "I2C_prueba_temp.h"

osTimerId_t id_timer__measureTemperature = NULL;
osThreadId_t id_thread__measureTemperature = NULL;

float temp_aux;
float calc;

static void timer__mesureTemperature_callback (void *no_argument) {
    osThreadFlagsSet(id_thread__measureTemperature, FLAG_TEMP);
}

void thread__measureTemperature (void *argument) {
    uint8_t nose;
    uint8_t buff[2];
    nose = I2C_Init_All();
    id_timer__measureTemperature = osTimerNew (timer__mesureTemperature_callback, osTimerPeriodic, NULL, NULL);
    osTimerStart(id_timer__measureTemperature, 100);
    for(;;) {
    osThreadFlagsWait(FLAG_TEMP, osFlagsWaitAny, osWaitForever);
    I2C_ReadRegisters(I2C_LINE_1, m_ADDR, REG_TEMP, buff, 2);
    }
}

void Init_temp_sensor(void) {
    id_thread__measureTemperature = osThreadNew (thread__measureTemperature, NULL, NULL);
}


