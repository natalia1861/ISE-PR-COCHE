#include "cmsis_os2.h"                          // CMSIS RTOS header file
#include "modo_sleep.h"
#include "nRF24L01_RX.h"
#include "adc.h"
#include "tm_stm32_nrf24l01.h"

RTC_HandleTypeDef RTCHandle;

void HAL_SYSTICK_Callback(void)
{
  HAL_IncTick();
}

void SleepMode_Measure(void)
{
  GPIO_InitTypeDef GPIO_InitStruct;

//  /* Disable USB Clock */
  __HAL_RCC_USB_OTG_FS_CLK_DISABLE();

  /* Disable Ethernet Clock */
  __HAL_RCC_ETH_CLK_DISABLE();

  /* Configure all GPIO as analog to reduce current consumption on non used IOs */
  /* Enable GPIOs clock */
//  __HAL_RCC_GPIOA_CLK_ENABLE();
//  __HAL_RCC_GPIOB_CLK_ENABLE();
//  __HAL_RCC_GPIOC_CLK_ENABLE();
//  __HAL_RCC_GPIOD_CLK_ENABLE();
//  __HAL_RCC_GPIOE_CLK_ENABLE();
//  __HAL_RCC_GPIOF_CLK_ENABLE();
//  __HAL_RCC_GPIOG_CLK_ENABLE();
//  __HAL_RCC_GPIOH_CLK_ENABLE();
//  __HAL_RCC_GPIOI_CLK_ENABLE();
//  __HAL_RCC_GPIOJ_CLK_ENABLE();
//  __HAL_RCC_GPIOK_CLK_ENABLE();

//  GPIO_InitStruct.Mode = GPIO_MODE_ANALOG;
//  GPIO_InitStruct.Speed = GPIO_SPEED_HIGH;
//  GPIO_InitStruct.Pull = GPIO_NOPULL;
//  GPIO_InitStruct.Pin = GPIO_PIN_All;
//  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct); 
//  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);
//  //HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);
//  //HAL_GPIO_Init(GPIOD, &GPIO_InitStruct);
//  HAL_GPIO_Init(GPIOE, &GPIO_InitStruct);
//  HAL_GPIO_Init(GPIOF, &GPIO_InitStruct);
//  //HAL_GPIO_Init(GPIOG, &GPIO_InitStruct);
//  HAL_GPIO_Init(GPIOH, &GPIO_InitStruct); 
//  HAL_GPIO_Init(GPIOI, &GPIO_InitStruct);
//  HAL_GPIO_Init(GPIOJ, &GPIO_InitStruct); 
//  HAL_GPIO_Init(GPIOK, &GPIO_InitStruct);

//  //Excluimos los pines de radiofrecuencia
//  GPIO_InitStruct.Pin = GPIO_PIN_All & ~(GPIO_PIN_10 | GPIO_PIN_11 | GPIO_PIN_12 | GPIO_PIN_0);
//  HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);
//  
//  GPIO_InitStruct.Pin = GPIO_PIN_All & ~(GPIO_PIN_6 | GPIO_PIN_7 );
//  HAL_GPIO_Init(GPIOD, &GPIO_InitStruct);
//  
//  GPIO_InitStruct.Pin = GPIO_PIN_All & ~(GPIO_PIN_3);
//  HAL_GPIO_Init(GPIOG, &GPIO_InitStruct);

//  /* Disable GPIOs clock */
  __HAL_RCC_GPIOA_CLK_DISABLE();
  __HAL_RCC_GPIOB_CLK_DISABLE();
  //__HAL_RCC_GPIOC_CLK_DISABLE();
  //__HAL_RCC_GPIOD_CLK_DISABLE();
  __HAL_RCC_GPIOE_CLK_DISABLE();
  __HAL_RCC_GPIOF_CLK_DISABLE();
  //__HAL_RCC_GPIOG_CLK_DISABLE();
  __HAL_RCC_GPIOH_CLK_DISABLE();
  __HAL_RCC_GPIOI_CLK_DISABLE();
  __HAL_RCC_GPIOJ_CLK_DISABLE();
  __HAL_RCC_GPIOK_CLK_DISABLE();

//  TM_NRF24L01_InitPins();
//  ADC1_pins_PC0_config();
//  Init_ADC1_consumo();

  /* Configure user Button */
  //BSP_PB_Init(BUTTON_KEY, BUTTON_MODE_EXTI);
  //BOTON AZUL
  __HAL_RCC_GPIOC_CLK_ENABLE();
//  __HAL_RCC_SYSCFG_CLK_ENABLE();
//  __HAL_RCC_SPI3_CLK_ENABLE();
//  __HAL_RCC_ADC1_CLK_ENABLE();
  
    /* Configure Button pin as input with External interrupt */
  GPIO_InitStruct.Mode = GPIO_MODE_IT_FALLING;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Pin = GPIO_PIN_13;
  HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);
  
    /* Enable and set Button EXTI Interrupt to the lowest priority */
  HAL_NVIC_SetPriority(EXTI15_10_IRQn, 2, 0);
  //HAL_NVIC_SetPriority(EXTI15_10_IRQn, 0x0F, 0x00);
  HAL_NVIC_EnableIRQ(EXTI15_10_IRQn);

  /* Suspend Tick increment to prevent wakeup by Systick interrupt. 
     Otherwise the Systick interrupt will wake up the device within 1ms (HAL time base) */
  HAL_SuspendTick();

  /* Request to enter SLEEP mode */
  HAL_PWR_EnterSLEEPMode(PWR_MAINREGULATOR_ON, PWR_SLEEPENTRY_WFI);

  /* Resume Tick interrupt if disabled prior to sleep mode entry */
  HAL_ResumeTick();
  
  /* Exit Ethernet Phy from LowPower mode */
  ETH_PhyExitFromPowerDownMode();
}

void ETH_PhyEnterPowerDownMode(void)
{
  ETH_HandleTypeDef heth;
  GPIO_InitTypeDef GPIO_InitStruct;
  uint32_t phyregval = 0; 
   
  /* This part of code is used only when the ETH peripheral is disabled 
	   when the ETH is used in the application this initialization code 
	   is called in HAL_ETH_MspInit() function  ***********************/
	
	/* Enable GPIO clocks*/
  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOC_CLK_ENABLE();
  
  /* Configure PA2: ETH_MDIO */
  GPIO_InitStruct.Speed = GPIO_SPEED_HIGH;
  GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL; 
  GPIO_InitStruct.Alternate = GPIO_AF11_ETH;
  GPIO_InitStruct.Pin = GPIO_PIN_1 | GPIO_PIN_2;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);
  
  /* Configure PC1: ETH_MDC */
  GPIO_InitStruct.Pin = GPIO_PIN_1;
  HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);
  
  /* Enable the ETH peripheral clocks */
  __HAL_RCC_ETH_CLK_ENABLE();
  
  /* Set ETH Handle parameters */
  heth.Instance = ETH;
  heth.Init.PhyAddress = LAN8742A_PHY_ADDRESS;

  /* Configure MDC clock: the MDC Clock Range configuration
	   depends on the system clock: 180Mhz/102 = 1.76MHz  */
  /* MDC: a periodic clock that provides the timing reference for 
	   the MDIO data transfer which shouldn't exceed the maximum frequency of 2.5 MHz.*/
  heth.Instance->MACMIIAR = (uint32_t)ETH_MACMIIAR_CR_Div102;

  /*****************************************************************/
  
  /* ETH PHY configuration in Power Down mode **********************/ 
  
  /* Read ETH PHY control register */
  HAL_ETH_ReadPHYRegister(&heth, PHY_BCR, &phyregval);
  
  /* Set Power down mode bit */
  phyregval |= PHY_POWERDOWN;
  
  /* Write new value to ETH PHY control register */
  HAL_ETH_WritePHYRegister(&heth, PHY_BCR, phyregval);
  
  /*****************************************************************/
  
  /* Disable periph CLKs */
  __HAL_RCC_GPIOA_CLK_DISABLE();
  __HAL_RCC_GPIOC_CLK_DISABLE();
  __HAL_RCC_ETH_CLK_DISABLE();
}

void ETH_PhyExitFromPowerDownMode(void)
{
   ETH_HandleTypeDef heth;
   GPIO_InitTypeDef GPIO_InitStruct;
   uint32_t phyregval = 0;
   
  /* ETH CLKs and GPIOs initialization ******************************/
  /* To be removed when the function is called from HAL_ETH_MspInit() when 
     exiting from Standby mode */
	
	/* Enable GPIO clocks*/
  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOC_CLK_ENABLE();
  
  /* Configure PA2 */
  GPIO_InitStruct.Speed = GPIO_SPEED_HIGH;
  GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL; 
  GPIO_InitStruct.Alternate = GPIO_AF11_ETH;
  GPIO_InitStruct.Pin = GPIO_PIN_1 | GPIO_PIN_2;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);
  
  /* Configure PC1*/
  GPIO_InitStruct.Pin = GPIO_PIN_1;
  HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);
  
  /* Enable ETH CLK */
  __HAL_RCC_ETH_CLK_ENABLE();
  /*****************************************************************/
  
  /* ETH PHY configuration to exit Power Down mode *****************/
  /* Set ETH Handle parameters */
  heth.Instance = ETH;
  heth.Init.PhyAddress = LAN8742A_PHY_ADDRESS;
  
  /* Configure MDC clock: the MDC Clock Range configuration
	   depends on the system clock: 180Mhz/102 = 1.76MHz  */
  /* MDC: a periodic clock that provides the timing reference for 
	   the MDIO data transfer which shouldn't exceed the maximum frequency of 2.5 MHz.*/
  heth.Instance->MACMIIAR = (uint32_t)ETH_MACMIIAR_CR_Div102; 
	
  /* Read ETH PHY control register */
  HAL_ETH_ReadPHYRegister(&heth, PHY_BCR, &phyregval);
  
  /* check if the PHY  is already in power down mode */
  if ((phyregval & PHY_POWERDOWN) != RESET)
  {
    /* Disable Power down mode */
    phyregval &= ~ PHY_POWERDOWN;
    
    /* Write value to ETH PHY control register */
    HAL_ETH_WritePHYRegister(&heth, PHY_BCR, phyregval);
  }
  /*****************************************************************/
}

HAL_StatusTypeDef HAL_ETH_ReadPHYRegister(ETH_HandleTypeDef *heth, uint16_t PHYReg, uint32_t *RegValue)
{
  uint32_t tmpreg1 = 0U;
  uint32_t tickstart = 0U;

  /* Check parameters */
  assert_param(IS_ETH_PHY_ADDRESS(heth->Init.PhyAddress));

  /* Check the ETH peripheral state */
  if(heth->State == HAL_ETH_STATE_BUSY_RD)
  {
    return HAL_BUSY;
  }
  /* Set ETH HAL State to BUSY_RD */
  heth->State = HAL_ETH_STATE_BUSY_RD;

  /* Get the ETHERNET MACMIIAR value */
  tmpreg1 = heth->Instance->MACMIIAR;

  /* Keep only the CSR Clock Range CR[2:0] bits value */
  tmpreg1 &= ~ETH_MACMIIAR_CR_MASK;

  /* Prepare the MII address register value */
  tmpreg1 |=(((uint32_t)heth->Init.PhyAddress << 11U) & ETH_MACMIIAR_PA); /* Set the PHY device address   */
  tmpreg1 |=(((uint32_t)PHYReg<<6U) & ETH_MACMIIAR_MR);                   /* Set the PHY register address */
  tmpreg1 &= ~ETH_MACMIIAR_MW;                                            /* Set the read mode            */
  tmpreg1 |= ETH_MACMIIAR_MB;                                             /* Set the MII Busy bit         */

  /* Write the result value into the MII Address register */
  heth->Instance->MACMIIAR = tmpreg1;

  /* Get tick */
  tickstart = HAL_GetTick();

  /* Check for the Busy flag */
  while((tmpreg1 & ETH_MACMIIAR_MB) == ETH_MACMIIAR_MB)
  {
    /* Check for the Timeout */
    if((HAL_GetTick() - tickstart ) > PHY_READ_TO)
    {
      heth->State= HAL_ETH_STATE_READY;

      /* Process Unlocked */
      __HAL_UNLOCK(heth);

      return HAL_TIMEOUT;
    }

    tmpreg1 = heth->Instance->MACMIIAR;
  }

  /* Get MACMIIDR value */
  *RegValue = (uint16_t)(heth->Instance->MACMIIDR);

  /* Set ETH HAL State to READY */
  heth->State = HAL_ETH_STATE_READY;

  /* Return function status */
  return HAL_OK;
}

HAL_StatusTypeDef HAL_ETH_WritePHYRegister(ETH_HandleTypeDef *heth, uint16_t PHYReg, uint32_t RegValue)
{
  uint32_t tmpreg1 = 0U;
  uint32_t tickstart = 0U;

  /* Check parameters */
  assert_param(IS_ETH_PHY_ADDRESS(heth->Init.PhyAddress));

  /* Check the ETH peripheral state */
  if(heth->State == HAL_ETH_STATE_BUSY_WR)
  {
    return HAL_BUSY;
  }
  /* Set ETH HAL State to BUSY_WR */
  heth->State = HAL_ETH_STATE_BUSY_WR;

  /* Get the ETHERNET MACMIIAR value */
  tmpreg1 = heth->Instance->MACMIIAR;

  /* Keep only the CSR Clock Range CR[2:0] bits value */
  tmpreg1 &= ~ETH_MACMIIAR_CR_MASK;

  /* Prepare the MII register address value */
  tmpreg1 |=(((uint32_t)heth->Init.PhyAddress<<11U) & ETH_MACMIIAR_PA); /* Set the PHY device address */
  tmpreg1 |=(((uint32_t)PHYReg<<6U) & ETH_MACMIIAR_MR);                 /* Set the PHY register address */
  tmpreg1 |= ETH_MACMIIAR_MW;                                           /* Set the write mode */
  tmpreg1 |= ETH_MACMIIAR_MB;                                           /* Set the MII Busy bit */

  /* Give the value to the MII data register */
  heth->Instance->MACMIIDR = (uint16_t)RegValue;

  /* Write the result value into the MII Address register */
  heth->Instance->MACMIIAR = tmpreg1;

  /* Get tick */
  tickstart = HAL_GetTick();

  /* Check for the Busy flag */
  while((tmpreg1 & ETH_MACMIIAR_MB) == ETH_MACMIIAR_MB)
  {
    /* Check for the Timeout */
    if((HAL_GetTick() - tickstart ) > PHY_WRITE_TO)
    {
      heth->State= HAL_ETH_STATE_READY;

      /* Process Unlocked */
      __HAL_UNLOCK(heth);

      return HAL_TIMEOUT;
    }

    tmpreg1 = heth->Instance->MACMIIAR;
  }

  /* Set ETH HAL State to READY */
  heth->State = HAL_ETH_STATE_READY;

  /* Return function status */
  return HAL_OK;

}
