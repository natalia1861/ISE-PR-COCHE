#include "gpio.h"
#include "nRF24L01_TX.h"
#include "app_main.h"
#include "joystick_control.h"

joy_pin_puerto joy_pin_pulsado; //variable global para guardar que pin ha sido pulsado y pasarselo al joystick

/***************PULSADOR****************/
void init_pulsador(void){
   static GPIO_InitTypeDef GPIO_InitStruct;

	__HAL_RCC_GPIOC_CLK_ENABLE();
	HAL_NVIC_EnableIRQ(EXTI15_10_IRQn);
  
    /*Configure GPIO pin : PC13 - USER BUTTON */
    GPIO_InitStruct.Pin = GPIO_PIN_13;
    GPIO_InitStruct.Mode = GPIO_MODE_IT_RISING;
    GPIO_InitStruct.Pull = GPIO_PULLDOWN;
    HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);
}

/***********nRF IRQ****************/
void init_nRF_IRQ (void)
{
    static GPIO_InitTypeDef GPIO_InitStruct;

    __HAL_RCC_GPIOG_CLK_ENABLE();
    HAL_NVIC_EnableIRQ(EXTI3_IRQn);
  
    /*Configure GPIO pin : PG3 - nRF IRQ*/
    GPIO_InitStruct.Pin = GPIO_PIN_3;
    GPIO_InitStruct.Mode = GPIO_MODE_IT_FALLING;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    HAL_GPIO_Init(GPIOG, &GPIO_InitStruct);
}

/***********Joystick****************/
void init_Joystick(void)
{
  static GPIO_InitTypeDef GPIO_InitStruct;
    
  __HAL_RCC_GPIOB_CLK_ENABLE();
  __HAL_RCC_GPIOE_CLK_ENABLE();
  __HAL_RCC_GPIOG_CLK_ENABLE();
  __HAL_RCC_GPIOF_CLK_ENABLE();
    
  HAL_NVIC_EnableIRQ(EXTI15_10_IRQn);

    //UP 
  GPIO_InitStruct.Mode = GPIO_MODE_IT_RISING;
  GPIO_InitStruct.Pull = GPIO_PULLDOWN;
  GPIO_InitStruct.Pin = GPIO_PIN_10;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);  
  
    //RIGHT 
  GPIO_InitStruct.Mode = GPIO_MODE_IT_RISING;
  GPIO_InitStruct.Pull = GPIO_PULLDOWN;
  GPIO_InitStruct.Pin = GPIO_PIN_11;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct); 

    //DOWN
  GPIO_InitStruct.Mode = GPIO_MODE_IT_RISING;
  GPIO_InitStruct.Pull = GPIO_PULLDOWN;
  GPIO_InitStruct.Pin = GPIO_PIN_12;
  HAL_GPIO_Init(GPIOF, &GPIO_InitStruct); 
  
    //LEFT
  GPIO_InitStruct.Mode = GPIO_MODE_IT_RISING;
  GPIO_InitStruct.Pull = GPIO_PULLDOWN;
  GPIO_InitStruct.Pin = GPIO_PIN_14;
  HAL_GPIO_Init(GPIOG, &GPIO_InitStruct);
  
    //CENTER
  GPIO_InitStruct.Mode = GPIO_MODE_IT_RISING;
  GPIO_InitStruct.Pull = GPIO_PULLDOWN;
  GPIO_InitStruct.Pin = GPIO_PIN_15;
  HAL_GPIO_Init(GPIOE, &GPIO_InitStruct);
}

void EXTI15_10_IRQHandler(void) {
    HAL_GPIO_EXTI_IRQHandler(GPIO_PIN_10);
	HAL_GPIO_EXTI_IRQHandler(GPIO_PIN_11);
	HAL_GPIO_EXTI_IRQHandler(GPIO_PIN_12);
	HAL_GPIO_EXTI_IRQHandler(GPIO_PIN_13);
	HAL_GPIO_EXTI_IRQHandler(GPIO_PIN_14);
	HAL_GPIO_EXTI_IRQHandler(GPIO_PIN_15);}

void EXTI3_IRQHandler(void) {
    HAL_GPIO_EXTI_IRQHandler(GPIO_PIN_3); //LIMPIA flag y llama a la callback
}

void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin){
  if (GPIO_Pin == GPIO_PIN_3) //Llamada a callback de nRF
  {
    HAL_GPIO_EXTI_Callback_NRF(GPIO_Pin);
  }
  
  if(GPIO_Pin==GPIO_PIN_13) //Boton azul
  {
    //*Mandamos comando para despertar al microcontrolador del modo bajo consumo*/
    osThreadFlagsSet(id_thread__app_main, FLAG__ENTER_LOW_POWER);
  }
  
  if(GPIO_Pin==GPIO_PIN_10) //UP 
  {
    joy_pin_pulsado.pin = GPIO_PIN_10;
    joy_pin_pulsado.port = GPIOB;
    osThreadFlagsSet(id_thread__joystick_control, FLAG_PULSACION);
  }
  
  if(GPIO_Pin==GPIO_PIN_11) //RIGHT
  {
    joy_pin_pulsado.pin = GPIO_PIN_11;
    joy_pin_pulsado.port = GPIOB;
    osThreadFlagsSet(id_thread__joystick_control, FLAG_PULSACION);  
  }
  
  if(GPIO_Pin==GPIO_PIN_12) //DOWN
  {
    joy_pin_pulsado.pin = GPIO_PIN_12;
    joy_pin_pulsado.port = GPIOE;
    osThreadFlagsSet(id_thread__joystick_control, FLAG_PULSACION);  
  }
  
  if(GPIO_Pin==GPIO_PIN_14) //LEFT
  {
    joy_pin_pulsado.pin = GPIO_PIN_14;
    joy_pin_pulsado.port = GPIOE;
    osThreadFlagsSet(id_thread__joystick_control, FLAG_PULSACION);  
  }
  
  if(GPIO_Pin==GPIO_PIN_15) //CENTER
  {
    joy_pin_pulsado.pin = GPIO_PIN_15;
    joy_pin_pulsado.port = GPIOE;
    osThreadFlagsSet(id_thread__joystick_control, FLAG_PULSACION);  
  }
}

void InitAllGPIOs (void)
{
    init_pulsador();
    init_nRF_IRQ();
    init_Joystick();
}
