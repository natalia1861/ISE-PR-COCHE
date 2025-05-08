#ifndef __GPIO_H_
#define __GPIO_H_

#include "stm32f4xx_hal.h"
#include "rtc.h"

typedef struct {
  uint16_t pin;
  GPIO_TypeDef * port;
} joy_pin_puerto;

extern joy_pin_puerto joy_pin_pulsado;

typedef enum {
    HAL_EXTI_Result_Ok = 0x00,
    HAL_EXTI_Result_Error,
    HAL_EXTI_Result_Not_Defined
} HAL_EXTI_Result_t;

void init_pulsador(void);
void init_nRF_IRQ (void);
void init_Joystick(void);
void InitAllGPIOs (void);

#endif
