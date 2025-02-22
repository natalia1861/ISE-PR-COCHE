#ifndef __THLCD_H
#define __THLCD_H

#include "cmsis_os2.h"                          // CMSIS RTOS header file
#include "Driver_SPI.h"
#include "stm32f4xx_hal.h"
#include "string.h"
#include "stdio.h"

void LCD_start(void);
void LCD_clean(void);
void LCD_write(uint8_t line, char a[]);

#endif
