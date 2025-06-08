#include <stdio.h>
#include "stm32f4xx_hal.h"
#include "adc.h"

/*Fichero comun para controlar los ADC del mando y del coche
 * En el mando se encuentra el ADC de presion (asociado a la galga). Utilizamos el ADC1 canal 13 pin PC0
 * En el coche se encuentra el ADC de consumo. Utilizamos el ADC1 canal 10 pin PC3.

 Se podria haber decidido usar el mismo ADC, canal y pin para ambos, pero se decidio asi­ para poder distinguir bien cada cosa.
*/

#define RESOLUTION_12B 4096U
#define VREF 3.3f

/**
  * @brief config the use of analog inputs ADC123_IN10 and ADC123_IN13 and enable ADC1 clock
  * @param None
  * @retval None
  */
 
//Definiciones para las marchas del pedal segun el valor recibido del ADC
#define NUM_MIN_MARCHAS     ADC_MARCHA_0
#define NUM_MAX_MARCHAS     ADC_MARCHA_2

#define MIN_VOLTIOS_ADC     0.0
#define MAX_VOLTIOS_ADC     3.3
#define SENSIBILITY         0.3

//Variables internas	
ADC_HandleTypeDef adchandle1;
ADC_HandleTypeDef adchandle2;

//Funciones internas
float ADC_getVoltage(ADC_HandleTypeDef *hadc, uint32_t Channel);

/* Read analog inputs */
uint16_t ADC_in (uint32_t channel) {
  int32_t valor = 0;

  if (channel == 0)
  {
    valor = 1243* ADC_getVoltage(&adchandle1, 10);
  }
  else if (channel == 1)
  {
      valor = 1243* ADC_getVoltage(&adchandle2, 13);
  }
  return ((uint16_t)valor);
}

//Inicializacioin de los pines del ADC
	/*PC0     ------> ADC1_IN10 POT 2 CONSUMO
    PC3     ------> ADC1_IN13 POT 1 PRESION
    */

void ADC1_pins_PC0_config(void)
{
    GPIO_InitTypeDef GPIO_InitStruct = {0};
	__HAL_RCC_ADC1_CLK_ENABLE();
	__HAL_RCC_GPIOC_CLK_ENABLE();
    
    GPIO_InitStruct.Pin = GPIO_PIN_0;
    GPIO_InitStruct.Mode = GPIO_MODE_ANALOG;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);
}

void ADC1_pins_PC3_config(void)
{
    GPIO_InitTypeDef GPIO_InitStruct = {0};
	__HAL_RCC_ADC1_CLK_ENABLE();
	__HAL_RCC_GPIOC_CLK_ENABLE();
    
    GPIO_InitStruct.Pin = GPIO_PIN_3;
    GPIO_InitStruct.Mode = GPIO_MODE_ANALOG;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);
}
void ADC1_pins_F429ZI_config(void)
{
    ADC1_pins_PC0_config();
    ADC1_pins_PC3_config();
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
    /** Configure for the selected ADC regular channel its corresponding rank in the sequencer and its sample time.*/
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
	
void Init_ADC1_consumo (void)
{
    ADC1_pins_PC0_config();
    ADC_Init_Single_Conversion(&adchandle1,ADC1);
}

void Init_ADC1_presion(void)
{
    ADC1_pins_PC3_config();
    ADC_Init_Single_Conversion(&adchandle2,ADC1);
}

//Funcion que devuelve directamente el consumo en mA
float getConsumo(void) //revisarNAK
{
    float corriente_Consumo =0; 

    //2.5V tenemos 150mA
	//adc_valor es x
    corriente_Consumo = ADC_in(CH0_CONSUMO)*0.15/2.5;
 	return corriente_Consumo;
	//printf("Corriente: %.2f\n", corriente_Consumo);
}

//Funcion que devuelve el pedal al que accedemos segun la presion
//IMPORTANTE: 3.3V es marcha 0, y 0V es marcha 2
marchas_t getPedal(void)
{
    float voltios_Pedal = 0; 
    float tramo;
    uint32_t marcha_calculada;
    marchas_t marcha;

  //Leemos del ADC el valor en bruto (por ej: 1.5V)
    voltios_Pedal = ADC_in(CH1_PRESION) * 3.3 / 3000;

    if (voltios_Pedal <= (MIN_VOLTIOS_ADC + SENSIBILITY))
        return ADC_MARCHA_2;  // 0 V

    if (voltios_Pedal >= (MAX_VOLTIOS_ADC - SENSIBILITY))
        return ADC_MARCHA_0;  // 3.3V

    // Dividimos el rango util en tramos (sabemos cuando voltaje ocupa cada marcha = 3.3/2 = 1.65V)
    tramo = (MAX_VOLTIOS_ADC - MIN_VOLTIOS_ADC) / (float)(NUM_MAX_MARCHAS - NUM_MIN_MARCHAS);

    // Normalizamos el voltaje al rango de tramos: en nuestro ejemplo: ((1.5 - 0)/1.65 = 0.9+0.5 = 1.4) -> marcha calculada = 1
    marcha_calculada = (uint32_t)(((voltios_Pedal - MIN_VOLTIOS_ADC) / tramo) + 0.5f); //sumamos 0.5 para aproximar siempre hacia arriba

    // Invertimos el indice de la marcha (2-1 = 1) -> marcha 1
    marcha = (marchas_t)((NUM_MAX_MARCHAS) - marcha_calculada);

    return marcha;
}



