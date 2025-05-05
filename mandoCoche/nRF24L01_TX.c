/**
 * Keil project example for NRF24L01+ using interrupts
 *
 * Transmitter code
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
/* Include core modules */
#include "stm32f4xx_hal.h"
#include "cmsis_os2.h"
#include "defines.h"
//#include "tm_stm32_disco.h"
//#include "tm_stm32_delay.h"
#include "tm_stm32_nrf24l01.h"
//#include "tm_stm32_usart.h"
#include "tm_stm32_exti.h"
#include <stdio.h>
#include <stdbool.h>

//control de leds
extern bool LED_Rrun;	//referenciado a HTTP_Server.c
extern bool LED_Grun;	//referenciado a HTTP_Server.c

/* My address */
uint8_t MyAddress[] = {
	0xE7,
	0xE7,
	0xE7,
	0xE7,
	0xE7
};
/* Receiver address */
uint8_t TxAddress[] = {
	0x7E,
	0x7E,
	0x7E,
	0x7E,
	0x7E
};

//Hilos y timers
osThreadId_t id_thread__RF_TX = NULL;

/* Data received and data for send */
uint8_t dataOut[32], dataIn[32];

/* Interrupt pin settings */
#define IRQ_PORT    GPIOA
#define IRQ_PIN     GPIO_PIN_10

/* NRF transmission status */
TM_NRF24L01_Transmit_Status_t transmissionStatus;
TM_NRF24L01_IRQ_t NRF_IRQ;


/* Buffer for strings */
char str[40];

void thread__test_transmissor_RF_TX(void *argument) 
{
	uint8_t printed = 0;
	uint32_t started_time = 0;

	/* Init system clock for maximum system speed */
	//TM_RCC_InitSystem();
	
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
	
	/* Set 2MBps data rate and -18dBm output power */
	TM_NRF24L01_SetRF(TM_NRF24L01_DataRate_2M, TM_NRF24L01_OutputPower_M18dBm);
	
	/* Set my address, 5 bytes */
	TM_NRF24L01_SetMyAddress(MyAddress);
	
	/* Set TX address, 5 bytes */
	TM_NRF24L01_SetTxAddress(TxAddress);
	
	/* Attach interrupt for NRF IRQ pin */
	TM_EXTI_Attach(IRQ_PORT, IRQ_PIN, TM_EXTI_Trigger_Falling);
	
	
	while (1)
    {
		/* Every 2 seconds */
		/* Fill data with something */
		sprintf((char *)dataOut, "abcdefghijklmnoszxABCDEFCBDA");
		
		/* Display on DEBUG */
		printf("pinging: ");
        
        started_time = HAL_GetTick();
		
		/* Transmit data, goes automatically to TX mode */
		TM_NRF24L01_Transmit(dataOut);
		
		/* Turn on led to indicate sending */
		LED_Grun = true;
		
		/* Set NRF state to sending */
		transmissionStatus = TM_NRF24L01_Transmit_Status_Sending;
		
		/* Reset printed flag */
		printed = 0;

        osDelay(2000);
		/* Check if status has changed */
		if (transmissionStatus != TM_NRF24L01_Transmit_Status_Sending && /*!< We are not sending anymore */
			!printed                                                     /*!< We didn't print status to user */
		) {
			/* Print time in ms */
            printf("pinging: %d", (started_time - HAL_GetTick()));
						
			/* Transmission was OK */
			if (transmissionStatus == TM_NRF24L01_Transmit_Status_Ok) {
				printf("Transmission OK");
			}
			
			/* Message LOST */
			if (transmissionStatus == TM_NRF24L01_Transmit_Status_Lost) {
				printf("Transmission Lost");
			}
			
			/* Set flag */
			printed = 1;
		}
	}
}

/* Interrupt handler */ //revisarNAK mover funcion a fichero generico de gestion de interrupciones (por ahora servira creo)
void TM_EXTI_Handler(uint16_t GPIO_Pin) 
{
	/* Check for proper interrupt pin */
	if (GPIO_Pin == IRQ_PIN) {
		/* Read interrupts */
		TM_NRF24L01_Read_Interrupts(&NRF_IRQ);
		
		/* Check if transmitted OK (received ack)*/
		if (NRF_IRQ.F.DataSent) { 		
			printf("Data Sent IRQ");		
			/* Save transmission status */
			transmissionStatus = TM_NRF24L01_Transmit_Status_Ok;
			
			/* Turn off led */
			LED_Grun = false;
			
			/* Go back to RX mode */
			TM_NRF24L01_PowerUpRx();
		}
		
		/* Check if max retransmission reached and last transmission failed */
		if (NRF_IRQ.F.MaxRT) {
			printf("Max RT IRQ");

			/* Save transmission status */
			transmissionStatus = TM_NRF24L01_Transmit_Status_Lost;
			
			/* Turn off led */
			LED_Grun = false;
			
			/* Go back to RX mode */
			TM_NRF24L01_PowerUpRx();
		}
		
		/* If data is ready on NRF24L01+ */
		if (NRF_IRQ.F.DataReady) { //una vez se fue a RX, si recibe datos
			printf("Data Ready IRQ");

			/* Get data from NRF24L01+ */
			TM_NRF24L01_GetData(dataIn);		
		}
	}
}

void Init_RF_TX(void) {
    id_thread__RF_TX = osThreadNew (thread__test_transmissor_RF_TX, NULL, NULL);
}
