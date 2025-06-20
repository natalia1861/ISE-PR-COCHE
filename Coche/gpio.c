#include "gpio.h"
#include "nRF24L01_RX.h"
#include "modo_sleep.h"
#include "app_main.h"
#include "cmsis_os2.h"                          // CMSIS RTOS header file
#include "distance_control.h"

	/**** Pulsador ****/
static GPIO_InitTypeDef  GPIO_InitStruct;

extern osThreadId_t id_thread__app_main;

void init_pulsador(void){
	__HAL_RCC_GPIOC_CLK_ENABLE();
	HAL_NVIC_EnableIRQ(EXTI15_10_IRQn);
  
  /*Configure GPIO pin : PC13 - USER BUTTON */
  GPIO_InitStruct.Pin = GPIO_PIN_13;
  GPIO_InitStruct.Mode = GPIO_MODE_IT_RISING;
  GPIO_InitStruct.Pull = GPIO_PULLDOWN;
  HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);
}

void init_nRF_IRQ (void)
{
    __HAL_RCC_GPIOG_CLK_ENABLE();
	HAL_NVIC_EnableIRQ(EXTI3_IRQn);
  
  /*Configure GPIO pin : PG3 - nRF IRQ*/
  GPIO_InitStruct.Pin = GPIO_PIN_3;
  GPIO_InitStruct.Mode = GPIO_MODE_IT_FALLING;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(GPIOG, &GPIO_InitStruct);
}

void EXTI15_10_IRQHandler(void) {
	  HAL_GPIO_EXTI_IRQHandler(GPIO_PIN_13);
}

void EXTI3_IRQHandler(void) {
    HAL_GPIO_EXTI_IRQHandler(GPIO_PIN_3); //Limpia flag y llama a la callback
}

void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin){
  if(GPIO_Pin==GPIO_PIN_13)
  {
    //Despertamos el micro por si acaso con el boton azul del coche. En principio se hace con comando de RF (el boton azul del mando).
    if(__HAL_PWR_GET_FLAG(PWR_FLAG_SB) != RESET)
    {
        __HAL_PWR_CLEAR_FLAG(PWR_FLAG_SB);
    }
    ETH_PhyExitFromPowerDownMode();
    
    //Inicializamos distancia porque tarda 6 segundos aprox en funcionar
    Init_SensorDistancia(); 
    
    //Volveos al estado normal del coche
    osThreadFlagsSet(id_thread__app_main,FLAG_STATE_NORMAL);
  }
  if (GPIO_Pin == GPIO_PIN_3) 
  {
    HAL_GPIO_EXTI_Callback_NRF(GPIO_Pin);  //Dentro de esta funcion se manda flag al hilo de control de RF para que lea el estado del IRQ (interrupcion)
  }
}

void InitAllGPIOs (void)
{
    init_pulsador();
    init_nRF_IRQ();
}

