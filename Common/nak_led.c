#include "nak_led.h"
#include "Board_LED.h"

#define LED_GREEN           0x00
#define LED_BLUE            0x01
#define LED_RED             0x02

void LED_GREEN_ON (void)
{
    LED_On(LED_GREEN);
}

void LED_GREEN_OFF(void)
{
    LED_Off(LED_GREEN);
}

void LED_BLUE_ON (void)
{
    LED_On(LED_BLUE);
}

void LED_BLUE_OFF(void)
{
    LED_Off(LED_BLUE);
}

void LED_RED_ON (void)
{
    LED_On(LED_RED);
}

void LED_RED_OFF(void)
{
    LED_Off(LED_RED);
}

//Inicializa los leds
void INITIALIZE_LEDS (void)
{
    LED_Initialize();
}


//Configura RGB
void RGB_mbed(void){
	
	 GPIO_InitTypeDef GPIO_InitStruct;
 __HAL_RCC_GPIOD_CLK_ENABLE();	
 
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_PULLDOWN;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
  
  GPIO_InitStruct.Pin = GPIO_PIN_11 | GPIO_PIN_12 | GPIO_PIN_13; //BGR
  HAL_GPIO_Init(GPIOD, &GPIO_InitStruct);
	
	HAL_GPIO_WritePin(GPIOD,GPIO_PIN_11, GPIO_PIN_SET);
  HAL_GPIO_WritePin(GPIOD,GPIO_PIN_12, GPIO_PIN_SET);
  HAL_GPIO_WritePin(GPIOD,GPIO_PIN_13, GPIO_PIN_SET);

}
