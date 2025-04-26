////SPI init
//extern ARM_DRIVER_SPI Driver_SPI4;
//static ARM_DRIVER_SPI* SPIdrv = &Driver_SPI4;


////Init SPI for W25Q16 memory
//static void W25Q16_Init_SPI(void){
//    __HAL_RCC_GPIOE_CLK_ENABLE();
//    
//    static GPIO_InitTypeDef GPIO_InitStruct_RFID;
//    /*CS*/    //SPI_CS -- SPI_B_NSS       PE11
//    __HAL_RCC_GPIOA_CLK_ENABLE();
//    GPIO_InitStruct_RFID.Mode = GPIO_MODE_OUTPUT_PP;
//    GPIO_InitStruct_RFID.Pull = GPIO_PULLUP;
//    GPIO_InitStruct_RFID.Speed = GPIO_SPEED_FREQ_HIGH;
//    GPIO_InitStruct_RFID.Pin = GPIO_PIN_11;
//    HAL_GPIO_Init(GPIOE, &GPIO_InitStruct_RFID);
//    HAL_GPIO_WritePin(GPIOE, GPIO_PIN_11, GPIO_PIN_SET);
//    
//    /*Reset*/   //SPI_MISO -- SPI_B_MISO  PA15
//    __HAL_RCC_GPIOA_CLK_ENABLE();
//    GPIO_InitStruct_RFID.Mode = GPIO_MODE_OUTPUT_PP;
//    GPIO_InitStruct_RFID.Pull = GPIO_PULLUP;
//    GPIO_InitStruct_RFID.Speed = GPIO_SPEED_FREQ_HIGH;
//    GPIO_InitStruct_RFID.Pin = GPIO_PIN_15;
//    HAL_GPIO_Init(GPIOA, &GPIO_InitStruct_RFID);
//    HAL_GPIO_WritePin(GPIOA, GPIO_PIN_15, GPIO_PIN_SET);
//    
//    /*SPI*/   
//    SPIdrv->Initialize(SPI_callback);
//    SPIdrv-> PowerControl(ARM_POWER_FULL);
//    SPIdrv-> Control(ARM_SPI_MODE_MASTER | ARM_SPI_CPOL1_CPHA1 | ARM_SPI_MSB_LSB | ARM_SPI_DATA_BITS (8), 1000000);
//    HAL_GPIO_WritePin(GPIOA, GPIO_PIN_15, GPIO_PIN_RESET);
//    osDelay(1);
//    HAL_GPIO_WritePin(GPIOA, GPIO_PIN_15, GPIO_PIN_SET);
//    osDelay(1000);
//  }
//  
//  static void SPI_callback(uint32_t event){
//      switch (event) {
//      case ARM_SPI_EVENT_TRANSFER_COMPLETE:
//          osThreadFlagsSet(TID_FLASH, TRANSFER_COMPLETE);
//          break;
//      case ARM_SPI_EVENT_DATA_LOST:
//          printf ("error");
//          /*  Occurs in slave mode when data is requested/sent by master
//              but send/receive/transfer operation has not been started
//              and indicates that data is lost. Occurs also in master mode
//              when driver cannot transfer data fast enough. */
//          __breakpoint(0);  /* Error: Call debugger or replace with custom error handling */
//          break;
//      case ARM_SPI_EVENT_MODE_FAULT:
//         printf ("error");
//          /*  Occurs in master mode when Slave Select is deactivated and
//              indicates Master Mode Fault. */
//          __breakpoint(0);  /* Error: Call debugger or replace with custom error handling */
//          break;
//      }
//  }

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

void thread__test_transmissor_RF(void) 
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
		
		/* Check if transmitted OK */
		if (NRF_IRQ.F.DataSent) {
			/* Save transmission status */
			transmissionStatus = TM_NRF24L01_Transmit_Status_Ok;
			
			/* Turn off led */
			LED_Grun = false;
			
			/* Go back to RX mode */
			TM_NRF24L01_PowerUpRx();
		}
		
		/* Check if max retransmission reached and last transmission failed */
		if (NRF_IRQ.F.MaxRT) {
			/* Save transmission status */
			transmissionStatus = TM_NRF24L01_Transmit_Status_Lost;
			
			/* Turn off led */
			LED_Grun = false;
			
			/* Go back to RX mode */
			TM_NRF24L01_PowerUpRx();
		}
		
		/* If data is ready on NRF24L01+ */
		if (NRF_IRQ.F.DataReady) { //creo que se refiere a SPI
			/* Get data from NRF24L01+ */
			TM_NRF24L01_GetData(dataIn);		
		}
	}
}
