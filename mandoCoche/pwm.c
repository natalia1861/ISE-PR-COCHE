#include "cmsis_os2.h"                          // CMSIS RTOS header file
#include "stm32f4xx_hal.h"
#include "pwm.h"
#include <stdio.h>
#include <string.h>
/*----------------------------------------------------------------------------
 *      												Thread PWM
 *---------------------------------------------------------------------------*/
 
//TIMER
TIM_HandleTypeDef htim; 
TIM_OC_InitTypeDef pwm_output;

void PWM_init(void);

void PWM_init(void){
	
	GPIO_InitTypeDef GPIO_InitStruct={0};
	__HAL_RCC_GPIOC_CLK_ENABLE();

	GPIO_InitStruct.Pin=GPIO_PIN_8;
	GPIO_InitStruct.Mode=GPIO_MODE_AF_PP; 
	GPIO_InitStruct.Pull=GPIO_PULLDOWN;
	GPIO_InitStruct.Alternate=GPIO_AF2_TIM3; 
	HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);
	
  //Timer 
  __HAL_RCC_TIM3_CLK_ENABLE(); 
  htim.Instance = TIM3;
  htim.Init.Prescaler = 167 ; 
  htim.Init.Period =  FREQ_TONO1 -1;  	
  htim.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
  //HAL_TIM_Base_Init(&htim1);

  HAL_TIM_PWM_Init(&htim);
  
  pwm_output.OCMode=TIM_OCMODE_PWM1;
  pwm_output.OCPolarity = TIM_OCPOLARITY_HIGH;
  pwm_output.OCFastMode = TIM_OCFAST_DISABLE;
	pwm_output.Pulse= (htim.Init.Period/2)-1;
	HAL_TIM_OC_Init(&htim);

	 //poner pwm en vez de oc en estas tres
	HAL_TIM_PWM_ConfigChannel(&htim,&pwm_output,TIM_CHANNEL_3);
	HAL_TIM_PWM_Start(&htim,TIM_CHANNEL_1);
}

void PWM_cambio(uint32_t frecuencia){
  htim.Init.Period = frecuencia-1; 
	pwm_output.Pulse= (htim.Init.Period/2)-1;
	//HAL_TIM_Base_Init(&htim1);
	HAL_TIM_PWM_Init(&htim);
}

void PWM_desactivar(){
	HAL_TIM_PWM_Stop(&htim,TIM_CHANNEL_3);
}

void PWM_activar(){
HAL_TIM_PWM_Start(&htim,TIM_CHANNEL_3);
}

void dist_Cerca(){
   osThreadFlagsSet(tid_PWM,0x04);
}

void dist_Media(){
osThreadFlagsSet(tid_PWM,0x08);
}
void dist_Lejos(){
osThreadFlagsSet(tid_PWM,0x10);
}
void tono_dist_pequena(){
	
	 PWM_activar();
   PWM_cambio(3000);
   HAL_Delay(150);
	 PWM_desactivar();	
	 HAL_Delay(150);
	 PWM_activar();
	 PWM_cambio(3000);
	 HAL_Delay(150);
	 PWM_desactivar();	
	 HAL_Delay(150);
	 PWM_activar();
	 PWM_cambio(3000);
	 HAL_Delay(150);
	 PWM_desactivar();
	
}

void tono_dist_med (){
	 PWM_activar();
   PWM_cambio(2000);
   HAL_Delay(150);
	 PWM_desactivar();	
	 HAL_Delay(200);
	 PWM_activar();
	 PWM_cambio(2000);
	 HAL_Delay(150);
	 PWM_desactivar();	
	 HAL_Delay(200);
	 PWM_activar();
	 PWM_cambio(2000);
	 HAL_Delay(150);
	 PWM_desactivar();
}

void tono_dist_gran (){
	PWM_activar();
   PWM_cambio(1000);
   HAL_Delay(150);
	 PWM_desactivar();	
	 HAL_Delay(500);
	 PWM_activar();
	 PWM_cambio(1000);
	 HAL_Delay(150);
	 PWM_desactivar();	
	 HAL_Delay(500);
	 PWM_activar();
	 PWM_cambio(1000);
	 HAL_Delay(150);
	 PWM_desactivar();
}