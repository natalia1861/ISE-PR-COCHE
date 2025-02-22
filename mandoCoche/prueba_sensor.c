#include "prueba_sensor.h"

osThreadId_t id_thread__sensor = NULL;
osTimerId_t id_timer__testMagnet = NULL;

static void timer__testMagnet_callback (void *no_argument) {
    osThreadFlagsSet(id_thread__sensor, FLAG_MP);
}

void thread__sennsor (void *argument) {
    static uint32_t status;
    status = AS5600_Start(AS5600_I2C_Line);
    if (status == AS5600_Magnet_Not_Present) {
        id_timer__testMagnet = osTimerNew (timer__testMagnet_callback, osTimerPeriodic, NULL, NULL);
        osTimerStart(id_timer__testMagnet, 100);
    }
    for(;;) { 
    osThreadFlagsWait(FLAG_MP, osFlagsWaitAny, osWaitForever);
    status = isMagnetPresent(AS5600_I2C_Line);
    }
}

void Init_sensor(void) {
    id_thread__sensor = osThreadNew (thread__sennsor, NULL, NULL);
}
