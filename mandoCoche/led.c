/*-----------------------------------------------------------------------------
 * Copyright (c) 2015-2021 Arm Limited (or its affiliates). All
 * rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *   1.Redistributions of source code must retain the above copyright
 *     notice, this list of conditions and the following disclaimer.
 *   2.Redistributions in binary form must reproduce the above copyright
 *     notice, this list of conditions and the following disclaimer in the
 *     documentation and/or other materials provided with the distribution.
 *   3.Neither the name of Arm nor the names of its contributors may be used
 *     to endorse or promote products derived from this software without
 *     specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL COPYRIGHT HOLDERS AND CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 *-----------------------------------------------------------------------------
 * Name:    LED_NUCLEO_F429ZI.c
 * Purpose: LED interface for NUCLEO-F429ZI board
 * Rev.:    1.0.0
 *----------------------------------------------------------------------------*/

#include "stm32f4xx_hal.h"
#include "led.h"

/* GPIO Pin configuration structure */
typedef struct {
  GPIO_TypeDef *port;
  uint16_t      pin;
  GPIO_PinState inactive_state;
  GPIO_PinState active_state;
} GPIO_PIN;


/* LED GPIO Pins */
static const GPIO_PIN LED_PIN[] = {
  { GPIOB, GPIO_PIN_0,  GPIO_PIN_RESET, GPIO_PIN_SET },
  { GPIOB, GPIO_PIN_7,  GPIO_PIN_RESET, GPIO_PIN_SET },
  { GPIOB, GPIO_PIN_14, GPIO_PIN_RESET, GPIO_PIN_SET },
	{ GPIOD, GPIO_PIN_11, GPIO_PIN_SET, GPIO_PIN_RESET },
  { GPIOD, GPIO_PIN_12, GPIO_PIN_SET, GPIO_PIN_RESET },
  { GPIOD, GPIO_PIN_13, GPIO_PIN_SET, GPIO_PIN_RESET }
};

#define LED_COUNT (sizeof(LED_PIN)/sizeof(GPIO_PIN))


/**
  \fn          int32_t LED_Initialize (void)
  \brief       Initialize LEDs
  \returns
   - \b  0: function succeeded
   - \b -1: function failed
*/
int32_t LED_Initialize (void) {
  GPIO_InitTypeDef GPIO_InitStruct;
  uint8_t          num;
  
  /* Enable LED GPIO Pins Clock */
  __HAL_RCC_GPIOB_CLK_ENABLE();
  __HAL_RCC_GPIOD_CLK_ENABLE();

  /* Configure GPIO LED pins */
  GPIO_InitStruct.Mode  = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull  = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_LOW;
  for (num = 0U; num < LED_COUNT; num++) {
    GPIO_InitStruct.Pin = LED_PIN[num].pin;
    HAL_GPIO_Init    (LED_PIN[num].port, &GPIO_InitStruct);
    HAL_GPIO_WritePin(LED_PIN[num].port, LED_PIN[num].pin, LED_PIN[num].inactive_state); 
  }

  return 0;
}

/**
  \fn          int32_t LED_Uninitialize (void)
  \brief       De-initialize LEDs
  \returns
   - \b  0: function succeeded
   - \b -1: function failed
*/
int32_t LED_Uninitialize (void) {
  uint8_t num;

  for (num = 0U; num < LED_COUNT; num++) {
    HAL_GPIO_DeInit(LED_PIN[num].port, LED_PIN[num].pin);
  }

  return 0;
}

/**
  \fn          int32_t LED_On (uint32_t num)
  \brief       Turn on requested LED
  \param[in]   num  LED number
  \returns
   - \b  0: function succeeded
   - \b -1: function failed
*/
int32_t LED_On (uint32_t num) {

  if (num >= LED_COUNT) {
    return -1;
  }

  HAL_GPIO_WritePin(LED_PIN[num].port, LED_PIN[num].pin, LED_PIN[num].active_state);

  return 0;
}

/**
  \fn          int32_t LED_Off (uint32_t num)
  \brief       Turn off requested LED
  \param[in]   num  LED number
  \returns
   - \b  0: function succeeded
   - \b -1: function failed
*/
int32_t LED_Off (uint32_t num) {

  if (num >= LED_COUNT) {
    return -1;
  }

  HAL_GPIO_WritePin(LED_PIN[num].port, LED_PIN[num].pin, LED_PIN[num].inactive_state);

  return 0;
}

/**
  \fn          int32_t LED_SetOut (uint32_t val)
  \brief       Write value to LEDs
  \param[in]   val  value to be displayed on LEDs
  \returns
   - \b  0: function succeeded
   - \b -1: function failed
*/
int32_t LED_SetOut (uint32_t val) {
  uint8_t num;

  for (num = 0U; num < LED_COUNT; num++) {
    if ((val & (1U << num)) != 0U) {
      LED_On (num);
    } else {
      LED_Off(num);
    }
  }

  return 0;
}

/**
  \fn          uint32_t LED_GetCount (void)
  \brief       Get number of LEDs
  \return      Number of available LEDs
*/
uint32_t LED_GetCount (void) {
  return LED_COUNT;
}
