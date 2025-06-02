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
//#include "tm_stm32_exti.h"
#include "gpio.h"
#include "nak_led.h"
#include "distance_control.h"
#include "app_main.h"
#include "servomotor.h"
#include "consumo_control.h"


/* Receiver address - direccion con la que se comunican mando y coche*/ 
uint8_t TxAddress[] = {
	0xE7,
	0xE7,
	0xE7,
	0xE7,
	0xE7
};
/* My address */
// uint8_t MyAddress[] = {
// 	0x7E,
// 	0x7E,
// 	0x7E,
// 	0x7E,
// 	0x7E
// };


//Hilos y timers
osThreadId_t id_thread__RF_RX = NULL;

/* Data received and data for send */
uint8_t dataIn[nRF_DATA_LENGTH] = {0};
uint8_t dataOut[nRF_DATA_LENGTH] = {0};

/* Interrupt pin settings */
#define IRQ_PORT    GPIOG
#define IRQ_PIN     GPIO_PIN_3

/* NRF transmission status */
TM_NRF24L01_Transmit_Status_t transmissionStatus;
TM_NRF24L01_IRQ_t NRF_IRQ;

//nRF Data
nRF_data_received_rx_t nRF_data_received_rx;

//DEBUG variables
HAL_EXTI_Result_t status_IRQ = HAL_EXTI_Result_Not_Defined;

void thread__transmissor_RF_RX(void *argument) 
{
    uint32_t flags;
    dataOut[1] = 0x02;
    dataOut[2] = 0x03;
    
    printf("Initializing...");

	/* Initialize NRF24L01+ on channel 15 and 32bytes of payload */
	/* By default 2Mbps data rate and 0dBm output power */
	/* NRF24L01 goes to RX mode by default */
	TM_NRF24L01_Init(15, 32);
	
	/* Set RF settings, Data rate to 2Mbps, Output power to -18dBm */
	TM_NRF24L01_SetRF(TM_NRF24L01_DataRate_2M, TM_NRF24L01_OutputPower_M18dBm);
	
	/* Set my address, 5 bytes */
	//TM_NRF24L01_SetMyAddress(MyAddress); //Se utilizaba para tener la transmision de TX por un lado y la de RX por otro

	/* Set TX address, and RX Pipe 0 address, 5 bytes */
	TM_NRF24L01_SetTxAddress(TxAddress);    // Se configura la dirección para recibir (RX_ADDR_P0) por la pipe0, que también se usará para devolver ACKs con o sin payload.
                                            // En modo PRX (recepcion), TX_ADDR no se utiliza para enviar ACKs, por lo que puede omitirse en el coche, ya que nunca entra en modo PTX (transmision).
	
	/* Enable interrupts for NRF24L01+ IRQ pin */
    init_nRF_IRQ();
        
    //Debug status
    printf("nRF initialized\n\n");
    
    printf("STATUS register: 0x%02X\n\n", TM_NRF24L01_GetStatus());         //Status value: 0x0E, direccion: 0x07 NRF24L01_REG_STATUS
    
    printf("FEATURE register: 0x%02X\n\n", TM_NRF24L01_ReadRegister(0x1D)); //Feature value: 0x06
    
    printf("DYNPD register: 0x%02X\n\n", TM_NRF24L01_ReadRegister(0x1C));   //DYNDP value: 0x3F
    
    while(1)
    {
        flags = osThreadFlagsWait(nRF_DATA_READY, osFlagsWaitAny, osWaitForever);
        
        if (flags & nRF_DATA_READY)
        {
            /* Read interrupts - read Status register*/
            TM_NRF24L01_Read_Interrupts(&NRF_IRQ);
		
            /* If data is ready on NRF24L01+*/
            //Si en modo RX: se activará si recibe datos normales. 
            //Si en modo TX: se activará si recibe ACK Payload .
            if (NRF_IRQ.F.DataReady) 
            {
                printf("\nIRQ: Data Ready IRQ\n");

                /* Get data from NRF24L01+ */
                TM_NRF24L01_GetData(dataIn, sizeof(dataIn));
                #ifndef TEST_RF
                switch (GET_NRF_COMMAND(dataIn)) //Se obtiene el comando (1er byte)
                {
                    case nRF_CMD__NORMAL_MODE:      //Pasamos al modo normal de funcionamiento (empleado para saber el sentido de los servos y desactivar distancia)
                        osThreadFlagsSet(id_thread__app_main, FLAG_STATE_NORMAL);
                        break;
                    case nRF_CMD__BACK_GEAR_MODE:   //Pasamos a modo marcha atras de funcionamiento (empleado para saber el sentido de los servos y activar distancia)
                        osThreadFlagsSet(id_thread__app_main, FLAG_STATE_BACK_GEAR);
                        break;
                    case nRF_CMD__LOW_POWER:        //Pasamos a modo bajo consumo
                        osThreadFlagsSet(id_thread__app_main, FLAG_STATE__LOW_POWER);
                        break;
                    case nRF_CMD__VELOCITY:         //Comando para cambiar velocidad de los servos
                        nRF_data_received_rx.velocidad = GET_NRF_AUX_DATA(dataIn); //Marchas. 0-2
                        setMotorSpeed((speed_marchas_t) nRF_data_received_rx.velocidad);
                        printf("Comando velocidad\n");
                        break;
                    case nRF_CMD__DIRECTION:        //Comando para cambiar la direccion del servo delantero
                        nRF_data_received_rx.direccion = GET_NRF_AUX_DATA(dataIn);
                        setServoAngle((float) ((float) nRF_data_received_rx.direccion / 100)); //El valor se obtiene como un float representado por uint16_t con 2 decimales
                        printf("Comando direccion\n");
                        break;

                    case nRF_CMD__ASK_DISTANCE:     //Comando para preguntar por la distancia
                        dataOut[nRF_DATA__COMMAND] = nRF_CMD__ASK_DISTANCE;                 //Se añade el comando de recibir consumo como respuesta
                        dataOut[nRF_DATA__AUX_DATA_LOW] = (uint8_t) distancia;              //Se añade el valor de distancia (low byte)
                        dataOut[nRF_DATA__AUX_DATA_HIGH] = (uint8_t) (distancia >> 8);      //Se añade el valor de distancia (high byte)
                        TM_NRF24L01_WriteAckPayload(NRF_IRQ.F.RX_P_NO, dataOut, sizeof(dataOut)); //Se añaden datos al ACK PAYLOAD
                        printf("Comando: ask Distancia\n");
                        break;
                    case nRF_CMD__ASK_CONSUMPTION:        //Comando para preguntar por el consumo
                        dataOut[nRF_DATA__COMMAND] = nRF_CMD__ASK_CONSUMPTION;
                        dataOut[nRF_DATA__AUX_DATA_LOW] = (uint8_t) consumption;            //Se añade el valor de distancia (low byte)
                        dataOut[nRF_DATA__AUX_DATA_HIGH] = (uint8_t) (consumption >> 8);    //Se añade el valor de distancia (high byte)
                        TM_NRF24L01_WriteAckPayload(NRF_IRQ.F.RX_P_NO, dataOut, sizeof(dataOut)); //Se añaden datos al ACK PAYLOAD
                        printf("Comando: ask Distancia\n");
                        break;
                    case nRF_CMD__RECEIVE_DISTANCE:
                    case nRF_CMD__RECIEVE_CONSUMPTION:
                    //No se realiza ninguna accion. Comandos utilizados para mandar el ack previamente cargado 
                    //Aunque en ambos se hagan lo mismo y se pueda englobar a 1 solo comando que sea "cargar ACK", se deja asi por claridad
                        break;
                }
                
                printf("Data received: [0]: %x [1]: %x [2]: %x\n", dataIn[0], dataIn[1], dataIn[2]);

                #else //TEST
                /* Write ACK Payload into TX FIFO */
                dataOut[0] = dataOut[0] +1; //Se añaden datos de payload (numero ascendente)
                printf("TX FIFO before: 0x%02X\n", TM_NRF24L01_TxFifoEmpty());
                TM_NRF24L01_WriteAckPayload(NRF_IRQ.F.RX_P_NO, dataOut, sizeof(dataOut));
    
//			    /* Send it back, NRF goes automatically to TX mode */
//			    TM_NRF24L01_Transmit(dataIn);
//			    
//			    /* Wait for data to be sent */
//			    do {
//			    	/* Wait till sending */
//			    	transmissionStatus = TM_NRF24L01_GetTransmissionStatus();
//			    } while (transmissionStatus == TM_NRF24L01_Transmit_Status_Sending);
                
                //Miramos si la cola esta llena o vacia
                printf("TX FIFO: 0x%02X\n", TM_NRF24L01_TxFifoEmpty());
                printf("After Transmission status: 0x%02X\n", TM_NRF24L01_GetStatus());
                #endif
            }
        
            /* If data is sent on NRF24L01+*/
                //Si en modo RX: se activara si envia correctamente el ACK (coche)
                //Si en modo TX: se activara si recibe ACK Payload. (mando)
            if (NRF_IRQ.F.DataSent) //He enviado correctamente un ack con payload
            {
                printf("IRQ: Data Sent: ACK with payload sent correctly\n");
            }
            
            //Maximo numero de reintentos - fallo en RF - se detectará porque no se enviará ACK asi que el mando se entera y lanza error
            if (NRF_IRQ.F.MaxRT)
            {
                printf("IRQ: Max RT\n");
                
            }
            
            //printf("AAAAA Status reg: 0x%02X\n", TM_NRF24L01_GetStatus());
            //printf("TX FIFO: 0x%02X\n", TM_NRF24L01_TxFifoEmpty());
            /* Clear interrupts */
            TM_NRF24L01_Clear_Interrupts();
            /* Clear RX FIFO*/
            TM_NRF24L01_Clear_RX_FIFO();
            /* Go back to RX mode */
            //TM_NRF24L01_PowerUpRx();	//No hace falta: como ya estamos en RX unicamente haria falta limpiar las interrupciones y limpiar RX FIFO
        }
    }
}

/* Interrupt handler */
void HAL_GPIO_EXTI_Callback_NRF(uint16_t GPIO_Pin) {
	/* Check for proper interrupt pin */
	if (GPIO_Pin == IRQ_PIN) 
    {
        //printf("\nNote: Interrupcion PG3 at: %d\n", HAL_GetTick());
        osThreadFlagsSet(id_thread__RF_RX, nRF_DATA_READY);     //Mandamos flag a hilo de control de RF de que hay datos disponibles
    }
}

void Init_RF_RX(void) {
    INITIALIZE_LEDS();
    id_thread__RF_RX = osThreadNew (thread__transmissor_RF_RX, NULL, NULL);
}
