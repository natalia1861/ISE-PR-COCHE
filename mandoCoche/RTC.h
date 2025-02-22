#ifndef __RTC_H
#define __RTC_H

#include <stdio.h>
#include <time.h>
#include "stm32f4xx_hal.h"
#include "rl_net.h"
#include "lcd.h"

#define FLAG_ALARMA 0x40

void init_RTC(void);
void RTC_set_Time(uint8_t day, uint8_t month, uint8_t year, uint8_t hora, uint8_t minutos, uint8_t segundos);
void RTC_getTime_Date(void);
void Init_alarma(int seg_alarm);
//void RTC_Set_Time_EXAMEN1(void);
#endif /* __RTC_H */
