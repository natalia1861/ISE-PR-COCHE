#include "gpio.h"
	/**** Pulsador ****/
static GPIO_InitTypeDef  GPIO_InitStruct;

void init_pulsador(void){
	__HAL_RCC_GPIOC_CLK_ENABLE();
	HAL_NVIC_EnableIRQ(EXTI15_10_IRQn);
  
  /*Configure GPIO pin : PC13 - USER BUTTON */
  GPIO_InitStruct.Pin = GPIO_PIN_13;
  GPIO_InitStruct.Mode = GPIO_MODE_IT_RISING;
  GPIO_InitStruct.Pull = GPIO_PULLDOWN;
  HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);
}

void EXTI15_10_IRQHandler(void) {
	HAL_GPIO_EXTI_IRQHandler(GPIO_PIN_13);
}

void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin){
  if(GPIO_Pin==GPIO_PIN_13){
    RTC_set_Time(1,1,0,0,0,0);
  }
}
