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
#include "nRF24L01_TX.h"
#include "cmsis_os2.h"
#include "defines.h"
#include "tm_stm32_nrf24l01.h"
#include "gpio.h"
#include <stdio.h>
#include <stdbool.h>
#include "lcd.h"
#include "app_main.h"
#include <string.h>
#include "errors.h"

/* My address */
uint8_t MyAddress[] = { //Address de envio y recepcion por pipe 0  del mando (para enviar y recibir los ack) -> debe coincidir con la de recepcion del coche
	0xE7,
	0xE7,
	0xE7,
	0xE7,
	0xE7
};
/* Receiver address */  //NO SE USA
// uint8_t TxAddress[] = {
// 	0x7E,
// 	0x7E,
// 	0x7E,
// 	0x7E,
// 	0x7E
// };

//Hilos y timers
osThreadId_t id_thread__SendData_RF_TX = NULL;
osThreadId_t id_thread__GetData_RF_TX = NULL;

//Queue
#define MAX_RF_MESS_QUEUE           5
osMessageQueueId_t id_queue__nRF_TX_Data = NULL;

uint32_t started_time = 0;

/* Data received and data for send */
uint8_t dataOut[nRF_DATA_LENGTH] = {0};
uint8_t dataIn[nRF_DATA_LENGTH] = {0};

/* Interrupt pin settings */
#define IRQ_PORT    GPIOG
#define IRQ_PIN     GPIO_PIN_3

/* NRF transmission status */
TM_NRF24L01_Transmit_Status_t transmissionStatus;
TM_NRF24L01_IRQ_t NRF_IRQ;

nRF_data_received_mando_t nRF_data_received_mando;

void thread__SendData_RF_TX(void *argument) 
{
    uint8_t printed = 0;
    nRF_data_transmitted_t nRF_data_transmitted;

    printf("Initializing...\n");
    
	/* Initialize NRF24L01+ on channel 15 and 32bytes of payload */
	/* By default 2Mbps data rate and 0dBm output power */
	/* NRF24L01 goes to RX mode by default */
	TM_NRF24L01_Init(15, 32);
	
	/* Set 2MBps data rate and -18dBm output power */
	TM_NRF24L01_SetRF(TM_NRF24L01_DataRate_2M, TM_NRF24L01_OutputPower_M18dBm);
	
	/* Set my address, 5 bytes */
	//TM_NRF24L01_SetMyAddress(MyAddress); //Se utilizaba para tener una pipe por donde transmitia el PTX y otra por donde transmitia PRX
	
	/* Set TX address, 5 bytes */
	TM_NRF24L01_SetTxAddress(MyAddress);  // Se configura la direcci�n para recibir (RX_ADDR_P0) por la pipe0, que tambi�n se usar� para devolver ACKs con o sin payload.
                                            // En modo PRX (recepcion), TX_ADDR no se utiliza para enviar ACKs, por lo que puede omitirse en el coche, ya que nunca entra en modo PTX (transmision).
                                            // En modo PTX (transmision), TX_ADDR es utilizado para enviar la informacion y RX_ADDR_P0 para recibirla por ACKs. Por lo que ambas deben coincidir (en el mando).
	
	/* Attach interrupt for NRF IRQ pin */
    init_nRF_IRQ();
    
    //Status right
    if (TM_NRF24L01_GetStatus() != 0x0E)
    {
        push_error(MODULE__RF, ERR_CODE__INITIALIZATION, 0); //revisarNAK error permanente. Por mucho que lo aceptes, se volvera a generar (hasta que la conexion vuelva).
    }
    
    //Debug status
    printf("nRF initialized\n\n");
    printf("STATUS register: 0x%02X\n\n", TM_NRF24L01_GetStatus());
    
    /* Fill data with something */
    sprintf((char *)dataOut, "987"); //Usado para el test
	
	while (1)
    {
        #ifdef TEST_RF //TEST
		/* Every 2 seconds */
        started_time = HAL_GetTick();
        
		/* Display on DEBUG */
		printf("\nPinging at %d: \n", started_time);
        LCD_write(2, "Pinging...          ");
        
		/* Transmit data, goes automatically to TX mode */
        TM_NRF24L01_Transmit(dataOut, sizeof(dataOut));
		
		/* Set NRF state to sending */
		transmissionStatus = TM_NRF24L01_Transmit_Status_Sending;
		
		/* Reset printed flag */
		printed = 0;
        
        printf("Status Before = 0x%02X\n", TM_NRF24L01_GetStatus());
        
        // Fuerza el handler manualmente (solo para probar)
        //TM_EXTI_Handler(GPIO_PIN_3);
        //NVIC_SetPendingIRQ(EXTI3_IRQn);
        osDelay(2000);

		/* Check if status has changed */
		if (transmissionStatus != TM_NRF24L01_Transmit_Status_Sending && /*!< We are not sending anymore */
			!printed)                                                     /*!< We didn't print status to user */
        {
			/* Print time in ms */
            printf("Check tranmision after : %d seconds\n", (HAL_GetTick() - started_time));
			printf("Status after IRQ = 0x%02X\n", TM_NRF24L01_GetStatus());

			/* Transmission was OK */
			if (transmissionStatus == TM_NRF24L01_Transmit_Status_Ok) {
				printf("Tramision result: OK\n\n");
                LCD_write(2, "Transmission OK     ");

			}
			
			/* Message LOST */
			if (transmissionStatus == TM_NRF24L01_Transmit_Status_Lost) {
				printf("Tramision result: Lost\n\n");
                LCD_write(2, "Transmission Lost  ");
			}
			
			/* Set flag */
			printed = 1;
		}
        else
        {
            printf("FATAL ERROR: NO IRQ\n\n");
            LCD_write(2, "no IRQ        ");
        }
        #else //Aplicacion
        
        //Esperamos a recibir un comando
        if (osMessageQueueGet(id_queue__nRF_TX_Data, &nRF_data_transmitted, NULL, osWaitForever) == osOK)
        {
            /* Display on DEBUG */
            printf("\nPinging at %d: \n", HAL_GetTick());
            printf("Command: %d\n", nRF_data_transmitted.command);
            printf("Auxiliar data: %d\n", nRF_data_transmitted.auxiliar_data);
            
            //Rellenamos los datos de salida con los del mensaje recibido
            dataOut[nRF_DATA__COMMAND]        = (uint8_t) (nRF_data_transmitted.command);
            dataOut[nRF_DATA__AUX_DATA_LOW]   = (uint8_t) (nRF_data_transmitted.auxiliar_data);         // Byte bajo
            dataOut[nRF_DATA__AUX_DATA_HIGH]  = (uint8_t) (nRF_data_transmitted.auxiliar_data >> 8);    // Byte alto

            //Transmitimos los datos
            TM_NRF24L01_Transmit(dataOut, sizeof(dataOut));

        }
        
        #endif
	}
}

void thread__GetData_RF_TX (void *no_argument)
{
    uint32_t flags;
    while (1)
    {
        flags = osThreadFlagsWait(nRF_DATA_READY, osFlagsWaitAny, osWaitForever);
        if (flags & nRF_DATA_READY)
        {
            #ifdef TEST_RF
                dataOut[0] = dataOut[0] + 1;
            #endif
            
            /* Read interrupts */
            TM_NRF24L01_Read_Interrupts(&NRF_IRQ);
		
            /* If data is sent on NRF24L01+*/
                //Si en modo RX: se activara si envia correctamente el ACK (coche)
                //Si en modo TX: se activara si recibe ACK Payload. (mando)

            if (NRF_IRQ.F.DataSent)  /* Check if transmitted OK (received ack)*/
            { 	
                //Si se recuperan comunicaciones
                if (transmissionStatus == TM_NRF24L01_Transmit_Status_Lost)	
                {
                    printf("IRQ: ACK Received\n");	

                    //Mandamos flag de recuperacion de comunciaciones para que vuelva la conexion
                    osThreadFlagsSet(id_thread__app_main , FLAG__RF_COMMS_RESTORED);

                    /* Save transmission status */
                    transmissionStatus = TM_NRF24L01_Transmit_Status_Ok;

                }
                
                //printf("STATUS FIFO in Data Sent %d\n", TM_NRF24L01_GetFIFOStatus());
            }
            
            /* Check if max retransmission reached and last transmission failed */
            if (NRF_IRQ.F.MaxRT) 
            {
                printf("IRQ: Max RT IRQ\n");
    
                /* Save transmission status */
                transmissionStatus = TM_NRF24L01_Transmit_Status_Lost;
                
                /* Mandamos flag de ERROR */
                #ifndef TEST_RF //Aplicacion
                    push_error(MODULE__RF, ERR_CODE__RF_COMMS_LOST, 0);                    
                //Error permanente. Por mucho que lo aceptes, se volvera a generar (hasta que la conexion vuelva).
                // Basicamente porque esta todo el rato intentando transmitir entonces se esta generando todo el rato de nuevo
                #endif
            }
            
            /* If data is ready on NRF24L01+*/
                //Si en modo RX: se activar� si recibe correctamente datos normales (coche)
                //Si en modo TX: se activar� si recibe correctamente ACK Payload (mando)
            if (NRF_IRQ.F.DataReady)  //Recibo ACKs de payload
            {
                printf("IRQ: Data Ready IRQ\n");
                //printf("STATUS FIFO before read %d\n", TM_NRF24L01_GetFIFOStatus());
                //printf("FIFO RX before read: %d\n", TM_NRF24L01_RxFifoEmpty());

                /* Get data from RX FIFO NRF24L01+ */
                TM_NRF24L01_GetData(dataIn, sizeof(dataIn));
                printf ("Data received: [0]: %x [1] %x [2] %x\n", dataIn[0], dataIn[1], dataIn[2]); //debug
                
                #ifndef TEST_RF //Aplicacion
                
                    if (GET_NRF_COMMAND(dataIn) == nRF_CMD__ASK_DISTANCE) //Comando de "pregunte por distancia" - es la respuesta con los datos de distancia
                    {
                        //Obtenemos el dato de distancia
                        nRF_data_received_mando.distancia = GET_NRF_AUX_DATA(dataIn);
                        
                        //Mandamos flag a app_main de que la distancia fue actualizada para mostrarse por lcd y pitido (en caso de que cambie)
                        osThreadFlagsSet(id_thread__app_main, FLAG__MOSTRAR_DISTANCIA);
                    }
                    
                    if (GET_NRF_COMMAND(dataIn) == nRF_CMD__ASK_CONSUMPTION) //Comando de "pregunte por consumo" - es la respuesta con los datos de consumo
                    {
                        //Obtenemos el dato de consumo
                        nRF_data_received_mando.consumo = GET_NRF_AUX_DATA(dataIn);
                        
                        //Mandamos flag a app_main de que el consumo fue actualizado para guardarse en flash y mandarse a web
                        osThreadFlagsSet(id_thread__app_main, FLAG__CONSUMO_EN_FLASH);
                    }
                
                #endif
                
                //printf("STATUS FIFO after read %d\n", TM_NRF24L01_GetFIFOStatus());
                //printf("FIFO RX after read: %d\n", TM_NRF24L01_RxFifoEmpty());
            }
        
            /* Go back to RX mode to get lower consumption*/
            TM_NRF24L01_PowerUpRx();
        }
    }
}

void Init_RF_TX(void) {
    if (id_queue__nRF_TX_Data == NULL)
        id_queue__nRF_TX_Data = osMessageQueueNew(MAX_RF_MESS_QUEUE, sizeof(nRF_data_transmitted_t), NULL);     //Cola de mensajes para gestionar el envio de RF
    
    if (id_thread__SendData_RF_TX == NULL)
        id_thread__SendData_RF_TX = osThreadNew (thread__SendData_RF_TX, NULL, NULL);             //Hilo para gestionar el envio de datos
    
    if (id_thread__GetData_RF_TX == NULL)
        id_thread__GetData_RF_TX = osThreadNew (thread__GetData_RF_TX, NULL, NULL);                //Hilo para gestionar la recepcion de datos

    //Gestion de errores
    if (id_queue__nRF_TX_Data == NULL)
        push_error (MODULE__RF, ERR_CODE__QUEUE, 0);

    if (id_thread__SendData_RF_TX == NULL)
        push_error (MODULE__RF, ERR_CODE__THREAD, 0);
    
    if (id_thread__GetData_RF_TX == NULL)
        push_error (MODULE__RF, ERR_CODE__THREAD, 1);
}


/* Interrupt handler */
void HAL_GPIO_EXTI_Callback_NRF(uint16_t GPIO_Pin)
{
	/* Check for proper interrupt pin */
	if (GPIO_Pin == IRQ_PIN) 
    {
        //printf("Note: Interrupcion PG3\n");
        osThreadFlagsSet(id_thread__GetData_RF_TX, nRF_DATA_READY); //Flag a hilo de control de RF
    }
}

