#ifndef __LCD_H
#define __LCD_H

#include "Driver_SPI.h"

typedef enum
{
    LCD_LINE__NO_LINE = 0, 
    LCD_LINE__ONE,
    LCD_LINE__TWO,
    LCD_LINE__THREE, 
    LCD_LINE__FOUR,
    
    LCD_LINE__MAX
} lineas_distancia_t;

//Funciones básicas
void LCD_clean(void);
void LCD_write(uint8_t line, char a[]);

//Funciones de lineas distancia
void LCD_mostrarLineasDistancia (lineas_distancia_t lineas);
void LCD_mostrarConsumo(uint8_t muestra, uint16_t consumo);

#endif /* _LCD_H */
