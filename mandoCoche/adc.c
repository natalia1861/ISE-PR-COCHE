#include <stdio.h>
#include "stm32f4xx_hal.h"
#include "adc.h"

#define RESOLUTION_12B 4096U
#define VREF 3.3f

/**
  * @brief config the use of analog inputs ADC123_IN10 and ADC123_IN13 and enable ADC1 clock
  * @param None
  * @retval None
  */
	
float corriente_Consumo =0; 
float voltios_Pedal =0; 
uint8_t marcha=0;
ADC_HandleTypeDef adchandle;
char consumo_S [80];
char marcha_S[80];

/* Read analog inputs */
uint16_t AD_in (uint32_t ch) {
  int32_t val = 0;

  if (ch == 0) {

    val = 1243* ADC_getVoltage(&adchandle, 10);
  }
  return ((uint16_t)val);
}
/* Read analog inputs */
uint16_t ADC_in (uint32_t channel) {
  int32_t valor = 0;

  if (channel == 1) {

    valor = 1243* ADC_getVoltage(&adchandle, 13);
	}
  return ((uint16_t)valor);
}




void ADC1_pins_F429ZI_config(){
	  GPIO_InitTypeDef GPIO_InitStruct = {0};
	__HAL_RCC_ADC1_CLK_ENABLE();
	__HAL_RCC_GPIOC_CLK_ENABLE();
	__HAL_RCC_GPIOA_CLK_ENABLE();
	/*PC0     ------> ADC1_IN10 POT 2
    PC3     ------> ADC1_IN13 POT 1
    */
    GPIO_InitStruct.Pin = GPIO_PIN_0;
    GPIO_InitStruct.Mode = GPIO_MODE_ANALOG;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);

    GPIO_InitStruct.Pin = GPIO_PIN_3;
    GPIO_InitStruct.Mode = GPIO_MODE_ANALOG;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);
		
		

  }
/**
  * @brief Initialize the ADC to work with single conversions. 12 bits resolution, software start, 1 conversion
  * @param ADC handle
	* @param ADC instance
  * @retval HAL_StatusTypeDef HAL_ADC_Init
  */
int ADC_Init_Single_Conversion(ADC_HandleTypeDef *hadc, ADC_TypeDef  *ADC_Instance)
{
	
   /** Configure the global features of the ADC (Clock, Resolution, Data Alignment and number of conversion)
  */
  hadc->Instance = ADC_Instance;
  hadc->Init.ClockPrescaler = ADC_CLOCK_SYNC_PCLK_DIV2;
  hadc->Init.Resolution = ADC_RESOLUTION_12B;
  hadc->Init.ScanConvMode = DISABLE;
  hadc->Init.ContinuousConvMode = DISABLE;
  hadc->Init.DiscontinuousConvMode = DISABLE;
  hadc->Init.ExternalTrigConvEdge = ADC_EXTERNALTRIGCONVEDGE_NONE;
  hadc->Init.ExternalTrigConv = ADC_SOFTWARE_START;
  hadc->Init.DataAlign = ADC_DATAALIGN_RIGHT;
  hadc->Init.NbrOfConversion = 1;
  hadc->Init.DMAContinuousRequests = DISABLE;
 hadc->Init.EOCSelection = ADC_EOC_SINGLE_CONV;
  if (HAL_ADC_Init(hadc) != HAL_OK)
  {
    return -1;
  }
 
	
	return 0;

}

/**
  * @brief Configure a specific channels ang gets the voltage in float type. This funtion calls to  HAL_ADC_PollForConversion that needs HAL_GetTick()
  * @param ADC_HandleTypeDef
	* @param channel number
	* @retval voltage in float (resolution 12 bits and VRFE 3.3
  */
float ADC_getVoltage(ADC_HandleTypeDef *hadc, uint32_t Channel)
	{
		ADC_ChannelConfTypeDef sConfig = {0};
		HAL_StatusTypeDef status;

		uint32_t raw = 0;
		float voltage = 0;
		 /** Configure for the selected ADC regular channel its corresponding rank in the sequencer and its sample time.
  */
  sConfig.Channel = Channel;
  sConfig.Rank = 1;
  sConfig.SamplingTime = ADC_SAMPLETIME_3CYCLES;
  if (HAL_ADC_ConfigChannel(hadc, &sConfig) != HAL_OK)
  {
    return -1;
  }
		
		HAL_ADC_Start(hadc);
		
		do (
			status = HAL_ADC_PollForConversion(hadc, 0)); //This funtions uses the HAL_GetTick(), then it only can be executed wehn the OS is running
		while(status != HAL_OK);
		
		raw = HAL_ADC_GetValue(hadc);
		
		voltage = raw*VREF/RESOLUTION_12B; 
		
		return voltage;

}
	
/* Example of using this code from a Thread 
void Thread (void *argument) {
  ADC_HandleTypeDef adchandle; //handler definition
	ADC1_pins_F429ZI_config(); //specific PINS configuration
	float value;
	ADC_Init_Single_Conversion(&adchandle , ADC1); //ADC1 configuration
  while (1) {
    
	  value=ADC_getVoltage(&adchandle , 10 ); //get values from channel 10->ADC123_IN10
		value=ADC_getVoltage(&adchandle , 13 );
		osDelay(1000);
   
  }
}
*/
void voltage_ADC(uint16_t adc_valor){
	   
		//2.5V tenemos 150mA
	  //adc_valor es x 	
	  corriente_Consumo = adc_valor*0.15/2.5;
	  printf("Corriente: %.2f\n", corriente_Consumo);
    
}

void voltage_pedales(uint16_t adc_valor){

	  //adc_valor es x 
	voltios_Pedal =  adc_valor*3.3/3000;
  printf("\nVoltios Pedales:%.2f ", voltios_Pedal);
	
	  if (voltios_Pedal < 1.1f) {
       marcha = 1;
			printf("\nTension:%.2f    Marcha:%d\n", voltios_Pedal,marcha);
			//sprintf(marcha_S, "%d",marcha);
    } else if (voltios_Pedal< 2.6f) {
       marcha = 2;
			printf("\nTension:%.2f    Marcha:%d\n", voltios_Pedal,marcha);	
			//sprintf(marcha_S, "%d",marcha);		
   } 
//		else {
//       marcha = 3;
//			printf("\nTension:%.2f    Marcha:%d\n", voltios_Pedal,marcha);
//		  //sprintf(marcha_S, "%d",marcha);
//    }
	 
}

