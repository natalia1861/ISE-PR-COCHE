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
#include "gpio.h"
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
uint8_t dataOut[5] = {0};

/* Interrupt pin settings */
#define IRQ_PORT    GPIOG
#define IRQ_PIN     GPIO_PIN_3

/* NRF transmission status */
TM_NRF24L01_Transmit_Status_t transmissionStatus;
TM_NRF24L01_IRQ_t NRF_IRQ;

//DEBUG variables
HAL_EXTI_Result_t status_IRQ = HAL_EXTI_Result_Not_Defined;

void thread__test_transmissor_RF_RX(void *argument) 
{
    printf("Initializing...");

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
    init_nRF_IRQ();
    
    //Debug status
    printf("nRF initialized\n\n");
    printf("STATUS register: 0x%02X\n\n", TM_NRF24L01_GetStatus());
    
    printf("FEATURE register: 0x%02X\n\n", TM_NRF24L01_ReadRegister(0x1D));
    
    printf("DYNPD register: 0x%02X\n\n", TM_NRF24L01_ReadRegister(0x1C));

	while (1) 
    {
        //Únicamente espera interrupciones. El hilo se eliminará
        //Se emplea para parpadear led azul para ver que este funcionando la placa
        //revisar_NAK quitar y crear tarea independiente
        LED_BLUE_ON();
        osDelay(200);
        LED_BLUE_OFF();
        osDelay(200);
	}
}

/* Interrupt handler */
void HAL_GPIO_EXTI_Callback_NRF(uint16_t GPIO_Pin) {
	/* Check for proper interrupt pin */
	if (GPIO_Pin == IRQ_PIN) 
    {
        printf("\nNote: Interrupcion PG3 at: %d\n", HAL_GetTick());
        
		/* Read interrupts */
		TM_NRF24L01_Read_Interrupts(&NRF_IRQ);
		
        /* If data is ready on NRF24L01+*/
            //Si en modo RX: se activará si recibe datos normales. 
            //Si en modo TX: se activará si recibe ACK Payload .
		if (NRF_IRQ.F.DataReady) {
			printf("IRQ: Data Ready IRQ\n");

			/* Get data from NRF24L01+ */
			TM_NRF24L01_GetData(dataIn);
			
			/* Start send */
            LED_GREEN_ON();
			
            dataOut[0] = dataOut[0]+1;
            TM_NRF24L01_WriteAckPayload(NRF_IRQ.F.RX_P_NO, dataOut, sizeof(dataOut));
            
//			/* Send it back, NRF goes automatically to TX mode */
//			TM_NRF24L01_Transmit(dataIn);
//			
//			/* Wait for data to be sent */
//			do {
//				/* Wait till sending */
//				transmissionStatus = TM_NRF24L01_GetTransmissionStatus();
//			} while (transmissionStatus == TM_NRF24L01_Transmit_Status_Sending);
			
            /* Send done */
            LED_GREEN_OFF();
			
			//Miramos si la cola esta llena o vacia
			printf("TX FIFO: 0x%02X\n", TM_NRF24L01_TxFifoEmpty());

			/* Go back to RX mode */
			//TM_NRF24L01_PowerUpRx();	
			printf("After Transmission status: 0x%02X\n", TM_NRF24L01_GetStatus());
		}
		
		/* Clear interrupts */
		TM_NRF24L01_Clear_Interrupts();
	}
}

void Init_RF_RX(void) {
    INITIALIZE_LEDS();
    id_thread__RF_RX = osThreadNew (thread__test_transmissor_RF_RX, NULL, NULL);
}
