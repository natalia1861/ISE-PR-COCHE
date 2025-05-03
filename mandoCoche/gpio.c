#include "gpio.h"
#include "nRF24L01_TX.h"
#include <stdio.h>

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
/**
 * @brief  Configura un pin GPIO para generar interrupciones externas (EXTI).
 * 
 * Esta funci�n configura un pin GPIO para ser utilizado como fuente de interrupci�n
 * externa en el STM32, habilitando la interrupci�n correspondiente en el controlador
 * de interrupciones del microcontrolador. El pin se configura con la opci�n de
 * disparo especificada (por ejemplo, flanco de subida o bajada) y sin resistencia de 
 * pull-up ni pull-down (**GPIO_NOPULL**), ya que la se�al de interrupci�n generada 
 * por el dispositivo conectado al pin debe ser gestionada de forma directa sin 
 * interferencia de resistencias internas.
 * 
 * @param GPIOx: Puerto GPIO en el cual se encuentra el pin (por ejemplo, GPIOA, GPIOB, etc.).
 * @param GPIO_Line: L�nea o pin del puerto GPIO a configurar (por ejemplo, GPIO_PIN_0, GPIO_PIN_1, etc.).
 * @param trigger: Modo de disparo para la interrupci�n (por ejemplo, GPIO_MODE_IT_RISING, GPIO_MODE_IT_FALLING, etc.).
 * 
 * @retval HAL_EXTI_Result_t: Resultado de la operaci�n (HAL_EXTI_Result_Ok si la configuraci�n fue exitosa,
 *                            HAL_EXTI_Result_Error si hubo alg�n error durante la configuraci�n).
 * 
 * @note  Es importante configurar el pin con **GPIO_NOPULL** en caso de que se utilice para
 *        interrupciones externas, ya que el dispositivo que genera la interrupci�n debe
 *        controlar el nivel l�gico del pin sin interferencia de las resistencias internas del 
 *        microcontrolador. En particular, se utiliza en configuraciones de pines de IRQ de 
 *        dispositivos como el **nRF24L01**.
 */
 
// Funci�n auxiliar para contar los ceros a la derecha de un n�mero (sin __builtin_ctz)
//int count_trailing_zeros(uint32_t x) {
//    if (x == 0) return 32;  // Si el n�mero es 0, tiene 32 ceros a la derecha
//    int n = 0;
//    while ((x & 1) == 0) {  // Mientras el bit menos significativo sea 0
//        x >>= 1;  // Desplazamos el n�mero una posici�n a la derecha
//        n++;      // Incrementamos el contador
//    }
//    return n;  // Retornamos el n�mero de ceros a la derecha
//}

//HAL_EXTI_Result_t HAL_EXTI_Attach(GPIO_TypeDef* GPIOx, uint16_t GPIO_Line, uint32_t trigger) {
//    GPIO_InitTypeDef GPIO_InitStruct = {0};
//    uint8_t pinsource, portsource;
//    IRQn_Type irqchannel;
//    
//    // Verificar si el usuario quiere inicializar m�s de un pin GPIO a la vez
//    if (!(GPIO_Line && !(GPIO_Line & (GPIO_Line - 1)))) {
//        uint8_t i;
//        // Verificar todos los pines
//        for (i = 0; i < 0x10; i++) {
//            if (GPIO_Line & (1 << i)) {
//                // Adjuntar un pin a la vez
//                if (HAL_EXTI_Attach(GPIOx, 1 << i, trigger) != HAL_EXTI_Result_Ok) {
//                    // Si uno falla, devolver error
//                    return HAL_EXTI_Result_Error;
//                }
//            }
//        }
//        // Devolver OK, todos los pines se han configurado correctamente
//        return HAL_EXTI_Result_Ok;
//    }

//    // Verificar si la l�nea ya est� en uso
//    if (
//        (EXTI->IMR & GPIO_Line) || /*!< Interrupci�n ya adjunta */
//        (EXTI->EMR & GPIO_Line)    /*!< Evento ya adjunto */
//    ) {
//        // Si la l�nea ya est� en uso, devolver error
//        return HAL_EXTI_Result_Error;
//    }
//    
//    // Habilitar el reloj del GPIO
//    if (GPIOx == GPIOA) __HAL_RCC_GPIOA_CLK_ENABLE();
//    else if (GPIOx == GPIOB) __HAL_RCC_GPIOB_CLK_ENABLE();
//    else if (GPIOx == GPIOC) __HAL_RCC_GPIOC_CLK_ENABLE();
//    else if (GPIOx == GPIOD) __HAL_RCC_GPIOD_CLK_ENABLE();
//    else if (GPIOx == GPIOE) __HAL_RCC_GPIOE_CLK_ENABLE();
//    else if (GPIOx == GPIOF) __HAL_RCC_GPIOF_CLK_ENABLE();
//    else if (GPIOx == GPIOG) __HAL_RCC_GPIOG_CLK_ENABLE();
//    else if (GPIOx == GPIOH) __HAL_RCC_GPIOH_CLK_ENABLE();
//    else if (GPIOx == GPIOI) __HAL_RCC_GPIOI_CLK_ENABLE();
//    
//    // Configurar el pin GPIO para la interrupci�n externa
//    GPIO_InitStruct.Pin = GPIO_Line;
//    GPIO_InitStruct.Mode = trigger;  // Ejemplo: GPIO_MODE_IT_FALLING, GPIO_MODE_IT_RISING
//    GPIO_InitStruct.Pull = GPIO_NOPULL;
//    HAL_GPIO_Init(GPIOx, &GPIO_InitStruct);
//    
//    // Determinar el canal IRQ correspondiente
//    pinsource = count_trailing_zeros(GPIO_Line);  // Obtener el n�mero del pin de origen
//    portsource = (uint32_t)GPIOx;
//    
//    // Seleccionar el canal IRQ adecuado
//    switch (pinsource) {
//        case 0: irqchannel = EXTI0_IRQn; break;
//        case 1: irqchannel = EXTI1_IRQn; break;
//        case 2: irqchannel = EXTI2_IRQn; break;
//        case 3: irqchannel = EXTI3_IRQn; break;
//        case 4: irqchannel = EXTI4_IRQn; break;
//        case 5: case 6: case 7: case 8: case 9:
//            irqchannel = EXTI9_5_IRQn; break;
//        case 10: case 11: case 12: case 13: case 14: case 15:
//            irqchannel = EXTI15_10_IRQn; break;
//        default:
//            return HAL_EXTI_Result_Error;
//    }
////    if (pinsource <= 4) {
////        irqchannel =  (IRQn_Type) EXTI0_IRQn + pinsource;
////    } else if (pinsource <= 9) {
////        irqchannel = EXTI9_5_IRQn;
////    } else {
////        irqchannel = EXTI15_10_IRQn;
////    }
//    
//    // Habilitar y configurar la interrupci�n
//    HAL_NVIC_SetPriority(irqchannel, 0, 0);
//    HAL_NVIC_EnableIRQ(irqchannel);
//    
//    // Desenmascarar la interrupci�n de la l�nea EXTI
//    EXTI->IMR |= GPIO_Line;

//    return HAL_EXTI_Result_Ok;
//}

void EXTI15_10_IRQHandler(void) {
	HAL_GPIO_EXTI_Callback(GPIO_PIN_13);
}

void EXTI3_IRQHandler(void) {
    printf("Interrupcion interna\n");
    HAL_GPIO_EXTI_IRQHandler(GPIO_PIN_3);   //LIMPIA flag y llama a la callback
    //HAL_GPIO_EXTI_Callback(GPIO_PIN_3);
}

void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin){
  if(GPIO_Pin==GPIO_PIN_13)
  {
     /*Despertamos el microcontrolador del modo bajo consumo*/
    //RTC_set_Time(1,1,0,0,0,0);
  }
  if (GPIO_Pin == GPIO_PIN_3) 
  {
    HAL_GPIO_EXTI_Callback_NRF(GPIO_Pin);  // o copia aqu� todo lo que ya tienes
  }
}
