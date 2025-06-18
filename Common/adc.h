#ifndef __ADC_H
#define __ADC_H

#include "stm32f4xx_hal.h"

typedef enum
{
    CH0_CONSUMO = 0,
    CH1_PRESION = 1
} ADC_channel_t;

typedef enum
{
    ADC_MARCHA_0 = 0,
    ADC_MARCHA_1,
    ADC_MARCHA_2
} marchas_t;

void ADC1_pins_F429ZI_config(void);
void ADC1_pins_PC0_config(void);
void Init_ADC1_consumo (void);
void Init_ADC1_presion(void);

marchas_t getPedal(void);           //Funcion que lee el ACD y te devuelve el pedal que se esta pulsando
float getConsumo(void);

#endif
