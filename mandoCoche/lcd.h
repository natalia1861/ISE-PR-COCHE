#ifndef __LCD_H
#define __LCD_H

#include "Driver_SPI.h"

#define LCD_MAX_CHARACTERS          17 //REVISAR COMPROBAR

typedef enum
{
    LCD_LINE__NO_LINE = 0, 
    LCD_LINE__ONE,
    LCD_LINE__TWO,
    LCD_LINE__THREE,  //LCD_MAX_LINES    
    
    LCD_LINE__MAX
} lineas_distancia_t;

#define LCD_MAX_LINES       LCD_LINE__THREE
#define LCD_MIN_LINES       LCD_LINE__NO_LINE

//Funciones b�sicas
void LCD_start(void);
void LCD_clean(void);
void LCD_write(uint8_t line, char a[]);

//Funciones de lineas distancia
void LCD_mostrarLineasDistancia (lineas_distancia_t lineas); //Muesta las lineas correspondientes
void LCD_mostrarConsumo(uint8_t muestra, float consumo);
#endif /* _LCD_H */
