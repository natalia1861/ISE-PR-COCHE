#ifndef __RTC_H
#define __RTC_H

#include <stdio.h>
#include <time.h>
#include "stm32f4xx_hal.h"
#include "rl_net.h"
#include "lcd.h"

typedef enum
{
    RTC_HOUR = 0,
    RTC_TIME,
    RTC_MAX
} time_hour_type_t;

extern char rtc_date_time[RTC_MAX][LCD_MAX_CHARACTERS+1];

void Init_RTC_Update (void); //Crea el hilo que ira actualizando el RTC

void RTC_set_Time(uint8_t day, uint8_t month, uint8_t year, uint8_t hora, uint8_t minutos, uint8_t segundos); //Permite poner una hora especifica
void RTC_getTime_Date(void); //Actualiza las variables globales de fecha y hora

#endif /* __RTC_H */
