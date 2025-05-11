#ifndef __GPIO_H_
#define __GPIO_H_

#include "stm32f4xx_hal.h"

typedef enum {
    HAL_EXTI_Result_Ok = 0x00,
    HAL_EXTI_Result_Error,
    HAL_EXTI_Result_Not_Defined
} HAL_EXTI_Result_t;

void InitAllGPIOs(void);
void init_pulsador(void);
void init_nRF_IRQ (void);
//HAL_EXTI_Result_t HAL_EXTI_Attach(GPIO_TypeDef* GPIOx, uint16_t GPIO_Line, uint32_t trigger);
#endif
