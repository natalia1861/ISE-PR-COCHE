/**
 * Keil project example for NRF24L01+ using interrupts
 *
 * Receiver code
 *
 * Before you start, select your target, on the right of the "Load" button
 *
 * @author    Tilen MAJERLE
 * @email     tilen@majerle.eu
 * @website   http://stm32f4-discovery.net
 * @ide       Keil uVision 5
 * @conf      PLL parameters are set in "Options for Target" -> "C/C++" -> "Defines"
 * @packs     STM32F4xx/STM32F7xx Keil packs are requred with HAL driver support
 * @stdperiph STM32F4xx/STM32F7xx HAL drivers required
 */

#include "nRF24L01_RX.h"
/* Include core modules */
#include "stm32f4xx_hal.h"
#include "cmsis_os2.h"                  // ::CMSIS:RTOS2
#include <stdio.h>
#include <stdbool.h>
/* Include my libraries here */
#include "defines.h"
//#include "tm_stm32_disco.h"
//#include "tm_stm32_delay.h"
#include "tm_stm32_nrf24l01.h"
//#include "tm_stm32_usart.h"
#include "tm_stm32_exti.h"

#include "nak_led.h"


/* Receiver address */
uint8_t TxAddress[] = {
	0xE7,
	0xE7,
	0xE7,
	0xE7,
	0xE7
};
/* My address */
uint8_t MyAddress[] = {
	0x7E,
	0x7E,
	0x7E,
	0x7E,
	0x7E
};


//Hilos y timers
osThreadId_t id_thread__RF_RX = NULL;

/* Data received and data for send */
uint8_t dataIn[32];

/* Interrupt pin settings */
#define IRQ_PORT    GPIOG
#define IRQ_PIN     GPIO_PIN_3

/* NRF transmission status */
TM_NRF24L01_Transmit_Status_t transmissionStatus;
TM_NRF24L01_IRQ_t NRF_IRQ;

void thread__test_transmissor_RF_RX(void *argument) 
{
    printf("Initializing...");
	/* Init system clock for maximum system speed */
	//TM_RCC_InitSystem();
	
	/* Init HAL layer */
	//HAL_Init();
	
	/* Init leds */
	//TM_DISCO_LedInit();
	
	/* Init button */
	//TM_DISCO_ButtonInit();
	
	/* Initialize USART, TX: PB6, RX: PB7 */
	//TM_USART_Init(USART1, TM_USART_PinsPack_2, 115200);
	
	/* Initialize NRF24L01+ on channel 15 and 32bytes of payload */
	/* By default 2Mbps data rate and 0dBm output power */
	/* NRF24L01 goes to RX mode by default */
	TM_NRF24L01_Init(15, 32);
	
	/* Set RF settings, Data rate to 2Mbps, Output power to -18dBm */
	TM_NRF24L01_SetRF(TM_NRF24L01_DataRate_2M, TM_NRF24L01_OutputPower_M18dBm);
	
	/* Set my address, 5 bytes */
	TM_NRF24L01_SetMyAddress(MyAddress);
	
	/* Set TX address, 5 bytes */
	TM_NRF24L01_SetTxAddress(TxAddress);
	
	/* Enable interrupts for NRF24L01+ IRQ pin */
	TM_EXTI_Attach(IRQ_PORT, IRQ_PIN, TM_EXTI_Trigger_Falling);

	while (1) {

	}
}

/* Interrupt handler */
void TM_EXTI_Handler(uint16_t GPIO_Pin) {
	/* Check for proper interrupt pin */
	if (GPIO_Pin == IRQ_PIN) {
		/* Read interrupts */
		TM_NRF24L01_Read_Interrupts(&NRF_IRQ);
		
		/* If data is ready on NRF24L01+ */
		if (NRF_IRQ.F.DataReady) {
            printf("Data Ready IRQ");

			/* Get data from NRF24L01+ */
			TM_NRF24L01_GetData(dataIn);
			
			/* Start send */
			//TM_DISCO_LedOn(LED_GREEN);
            LED_GREEN_ON();
			
			/* Send it back, NRF goes automatically to TX mode */
			TM_NRF24L01_Transmit(dataIn);
			
			/* Wait for data to be sent */
			do {
				/* Wait till sending */
				transmissionStatus = TM_NRF24L01_GetTransmissionStatus();
			} while (transmissionStatus == TM_NRF24L01_Transmit_Status_Sending);
			
            LED_GREEN_OFF();
			/* Send done */
			//TM_DISCO_LedOff(LED_GREEN);
			
			/* Go back to RX mode */
			TM_NRF24L01_PowerUpRx();		
		}
		
		/* Clear interrupts */
		TM_NRF24L01_Clear_Interrupts();
	}
}

void Init_RF_RX(void) {
    LED_Initialize();
    id_thread__RF_RX = osThreadNew (thread__test_transmissor_RF_RX, NULL, NULL);
}
