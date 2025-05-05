#ifndef __LCD_H
#define __LCD_H

#include "Driver_SPI.h"
#include "main.h"
#include "stdio.h"
#include "string.h"

# define PIN_RESET GPIO_PIN_6 /*PA6*/
# define PIN_A0  GPIO_PIN_13 /*PF13*/
# define PIN_CS  GPIO_PIN_14 /*PD14*/

extern char mensajeL1[100];
extern char mensajeL2[100];

extern ARM_DRIVER_SPI Driver_SPI1;

extern unsigned char buffer[512];

void delay(uint32_t n_microsegundos);
void LCD_reset(void);
void LCD_init(void);
void LCD_wr_data(unsigned char data);
void LCD_wr_cmd(unsigned char cmd);
void LCD_update_L1(void);
void LCD_update_L2(void);
void LCD_update_L3(void);
	void LCD_update_L4(void);
void symbolToLocalBuffer_L1(uint8_t symbol);
void symbolToLocalBuffer_L2(uint8_t symbol);
void symbolToLocalBuffer(uint8_t line, uint8_t symbol);
void escribirLinea(uint8_t line, char* mensaje);
void limpiarLCD(void);
void rellenarLCD_L1(void);
void rellenarLCD_L2(void);
void rellenarLCD_L3(void);
void rellenarLCD_L4(void);
#endif /* _LCD_H */
