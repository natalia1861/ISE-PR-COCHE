/**	
 * |----------------------------------------------------------------------
 * | Copyright (c) 2016 Tilen Majerle
 * |  
 * | Permission is hereby granted, free of charge, to any person
 * | obtaining a copy of this software and associated documentation
 * | files (the "Software"), to deal in the Software without restriction,
 * | including without limitation the rights to use, copy, modify, merge,
 * | publish, distribute, sublicense, and/or sell copies of the Software, 
 * | and to permit persons to whom the Software is furnished to do so, 
 * | subject to the following conditions:
 * | 
 * | The above copyright notice and this permission notice shall be
 * | included in all copies or substantial portions of the Software.
 * | 
 * | THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * | EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES
 * | OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE
 * | AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT
 * | HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
 * | WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING 
 * | FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
 * | OTHER DEALINGS IN THE SOFTWARE.
 * |----------------------------------------------------------------------
 */
#include "tm_stm32_nrf24l01.h"
#include <stdio.h>

/* NRF24L01+ registers*/
#define NRF24L01_REG_CONFIG			0x00	//Configuration Register
#define NRF24L01_REG_EN_AA			0x01	//Enable �Auto Acknowledgment� Function
#define NRF24L01_REG_EN_RXADDR		0x02	//Enabled RX Addresses
#define NRF24L01_REG_SETUP_AW		0x03	//Setup of Address Widths (common for all data pipes)
#define NRF24L01_REG_SETUP_RETR		0x04	//Setup of Automatic Retransmission
#define NRF24L01_REG_RF_CH			0x05	//RF Channel
#define NRF24L01_REG_RF_SETUP		0x06	//RF Setup Register	
#define NRF24L01_REG_STATUS			0x07	//Status Register
#define NRF24L01_REG_OBSERVE_TX		0x08	//Transmit observe register
#define NRF24L01_REG_RPD			0x09	
#define NRF24L01_REG_RX_ADDR_P0		0x0A	//Receive address data pipe 0. 5 Bytes maximum length.
#define NRF24L01_REG_RX_ADDR_P1		0x0B	//Receive address data pipe 1. 5 Bytes maximum length.
#define NRF24L01_REG_RX_ADDR_P2		0x0C	//Receive address data pipe 2. Only LSB
#define NRF24L01_REG_RX_ADDR_P3		0x0D	//Receive address data pipe 3. Only LSB
#define NRF24L01_REG_RX_ADDR_P4		0x0E	//Receive address data pipe 4. Only LSB
#define NRF24L01_REG_RX_ADDR_P5		0x0F	//Receive address data pipe 5. Only LSB
#define NRF24L01_REG_TX_ADDR		0x10	//Transmit address. Used for a PTX device only
#define NRF24L01_REG_RX_PW_P0		0x11	
#define NRF24L01_REG_RX_PW_P1		0x12	
#define NRF24L01_REG_RX_PW_P2		0x13	
#define NRF24L01_REG_RX_PW_P3		0x14	
#define NRF24L01_REG_RX_PW_P4		0x15	
#define NRF24L01_REG_RX_PW_P5		0x16	
#define NRF24L01_REG_FIFO_STATUS	0x17	//FIFO Status Register
#define NRF24L01_REG_DYNPD			0x1C	//Enable dynamic payload length
#define NRF24L01_REG_FEATURE		0x1D

/* Registers default values */
#define NRF24L01_REG_DEFAULT_VAL_CONFIG			0x08	
#define NRF24L01_REG_DEFAULT_VAL_EN_AA			0x3F	
#define NRF24L01_REG_DEFAULT_VAL_EN_RXADDR		0x03	
#define NRF24L01_REG_DEFAULT_VAL_SETUP_AW		0x03	
#define NRF24L01_REG_DEFAULT_VAL_SETUP_RETR		0x03	
#define NRF24L01_REG_DEFAULT_VAL_RF_CH			0x02	
#define NRF24L01_REG_DEFAULT_VAL_RF_SETUP		0x0E	
#define NRF24L01_REG_DEFAULT_VAL_STATUS			0x0E	
#define NRF24L01_REG_DEFAULT_VAL_OBSERVE_TX		0x00	
#define NRF24L01_REG_DEFAULT_VAL_RPD			0x00
#define NRF24L01_REG_DEFAULT_VAL_RX_ADDR_P0_0	0xE7
#define NRF24L01_REG_DEFAULT_VAL_RX_ADDR_P0_1	0xE7
#define NRF24L01_REG_DEFAULT_VAL_RX_ADDR_P0_2	0xE7
#define NRF24L01_REG_DEFAULT_VAL_RX_ADDR_P0_3	0xE7
#define NRF24L01_REG_DEFAULT_VAL_RX_ADDR_P0_4	0xE7
#define NRF24L01_REG_DEFAULT_VAL_RX_ADDR_P1_0	0xC2
#define NRF24L01_REG_DEFAULT_VAL_RX_ADDR_P1_1	0xC2
#define NRF24L01_REG_DEFAULT_VAL_RX_ADDR_P1_2	0xC2
#define NRF24L01_REG_DEFAULT_VAL_RX_ADDR_P1_3	0xC2
#define NRF24L01_REG_DEFAULT_VAL_RX_ADDR_P1_4	0xC2
#define NRF24L01_REG_DEFAULT_VAL_RX_ADDR_P2		0xC3	
#define NRF24L01_REG_DEFAULT_VAL_RX_ADDR_P3		0xC4	
#define NRF24L01_REG_DEFAULT_VAL_RX_ADDR_P4		0xC5
#define NRF24L01_REG_DEFAULT_VAL_RX_ADDR_P5		0xC6
#define NRF24L01_REG_DEFAULT_VAL_TX_ADDR_0		0xE7
#define NRF24L01_REG_DEFAULT_VAL_TX_ADDR_1		0xE7
#define NRF24L01_REG_DEFAULT_VAL_TX_ADDR_2		0xE7
#define NRF24L01_REG_DEFAULT_VAL_TX_ADDR_3		0xE7
#define NRF24L01_REG_DEFAULT_VAL_TX_ADDR_4		0xE7
#define NRF24L01_REG_DEFAULT_VAL_RX_PW_P0		0x00
#define NRF24L01_REG_DEFAULT_VAL_RX_PW_P1		0x00
#define NRF24L01_REG_DEFAULT_VAL_RX_PW_P2		0x00
#define NRF24L01_REG_DEFAULT_VAL_RX_PW_P3		0x00
#define NRF24L01_REG_DEFAULT_VAL_RX_PW_P4		0x00
#define NRF24L01_REG_DEFAULT_VAL_RX_PW_P5		0x00
#define NRF24L01_REG_DEFAULT_VAL_FIFO_STATUS	0x11
#define NRF24L01_REG_DEFAULT_VAL_DYNPD			0x00
#define NRF24L01_REG_DEFAULT_VAL_FEATURE		0x00

//Registers masks information

/* Configuration register - NRF24L01_REG_CONFIG */
#define NRF24L01_MASK_RX_DR		6 	/*Low active: Mask interrupt caused by RX_DR*/
#define NRF24L01_MASK_TX_DS		5 	/*Low active: Mask interrupt caused by TX_DS*/
#define NRF24L01_MASK_MAX_RT	4 	/*Low active: Mask interrupt caused by MAX_RT*/
#define NRF24L01_EN_CRC			3 	/*Enable CRC. Forced high if one of the bits in EN_AA is high*/
#define NRF24L01_CRCO			2 	/*CRC encoding scheme: 0: 1 byte, 1: 2 bytes*/
#define NRF24L01_PWR_UP			1 	/*1: Power UP, 0: Power down*/
#define NRF24L01_PRIM_RX		0 	/*RX/TX contol. 1: PRX, 0: PTX*/

/* Enable auto acknowledgment - NRF24L01_REG_EN_AA */
#define NRF24L01_ENAA_P5		5  /*Enable auto acknowledment data pipe 5*/
#define NRF24L01_ENAA_P4		4  /* ...                      data pipe 4*/
#define NRF24L01_ENAA_P3		3
#define NRF24L01_ENAA_P2		2
#define NRF24L01_ENAA_P1		1
#define NRF24L01_ENAA_P0		0

/* Enable rx addresses - NRF24L01_REG_EN_RXADDR */
#define NRF24L01_ERX_P5			5  /*Enable data pipe 5*/
#define NRF24L01_ERX_P4			4  /* ...   data pipe 4*/
#define NRF24L01_ERX_P3			3
#define NRF24L01_ERX_P2			2
#define NRF24L01_ERX_P1			1
#define NRF24L01_ERX_P0			0

/* Setup of address width - NRF24L01_REG_SETUP_AW */
#define NRF24L01_AW				0 //2 bits, 00: illegal, 01: 3 bytes, 10: 4 bytes, 11: 5 bytes

/* Setup of Automatic Retransmission - NRF24L01_REG_SETUP_RETR*/
#define NRF24L01_ARD			4 //4 bits, 0000: 250 us, 0001: 500 us, ..., 1111: 4000 us 
#define NRF24L01_ARC			0 //4 bits, 0000: re-transmits disabled, 0001: 1 re-transmits, 0010: 2 re-transmits, ..., 1111: 15 re-transmits

/* Setup of RF Channel - NRF24L01_REG_RF_CH*/
#define NRF24L01_RF_CH			0 //7 bits, Sets the frequency channel nRF24L01 operates on

/* RF setup register - NRF24L01_REG_RF_SETUP */
#define NRF24L01_PLL_LOCK		4 //Force PLL lock signal. Only used in test
#define NRF24L01_RF_DR_LOW		5 
#define NRF24L01_RF_DR_HIGH		3 
#define NRF24L01_RF_DR			3 //Air data rate, 0: 1Mbps, 1: 2Mbps
#define NRF24L01_RF_PWR			1 //2 bits. Set RF output power 00: -18dbm, 01: -12dbm, 10: -6dbm, 11: 0 dbm
#define NRF24L01_RF_HCURR		0 //Setup LNA gain

/* General status register - NRF24L01_REG_STATUS */
#define NRF24L01_RX_DR			6  //Data ready RX FIFO interrupt. Asserted when new data arrives in RX FIFO
#define NRF24L01_TX_DS			5  //Data sent TX FIFO interrupt. Asserted when packet transmitted on TX. If AUTO_ACK is activated, this bit is set high only when ACK is received.
#define NRF24L01_MAX_RT			4  //Maximum number of TX retransmits interrupt. 
#define NRF24L01_RX_P_NO		1  //3 bits. Data pipe number for the payload avaiable for reading from RX FIFO.
#define NRF24L01_TX_FULL		0  //TX FIFO full flag. 1: FULL, 0: Available locations in TX FIFO.

/* Transmit observe register - NRF24L01_REG_OBSERVE_TX*/
#define NRF24L01_PLOS_CNT		4 //4 bits, counts lost packets. Counter resets by writing to RF_CH
#define NRF24L01_ARC_CNT		0 //4 bits, count retransmitted packets. Counter reset when a transmission of a new packet starts.

/* Carrier Detect register - NRF24L01_REG_RPD*/
#define NRF24L01_CD				0 //1 bit, Carrier detect (Only read) - use only when MAX_RT is reccurrent

/* FIFO status - NRF24L01_REG_FIFO_STATUS*/
#define NRF24L01_TX_REUSE		6 //Reuse last transmitted data packet if set high. The packet is repeatedly retransmitted as long as CE is high.
                                  //TX_REUSE is set by the SPI command REUSE_TX_PL, and is reset by the SPI commands W_TX_PAYLOAD or FLUSH TX
#define NRF24L01_FIFO_FULL		5 //TX FIFO full flag. 1: TX FIFO full. 0: Available locations in TX FIFO.
#define NRF24L01_TX_EMPTY		4 //TX FIFO empty flag. 1: TX FIFO empty. 0: Data in TX FIFO.
#define NRF24L01_RX_FULL		1 //RX FIFO full flag. 1: RX FIFO full. 0: Available locations in RX FIFO.
#define NRF24L01_RX_EMPTY		0 //RX FIFO empty flag. 1: RX FIFO empty. 0: Data in RX FIFO.

/*Dynamic length - NRF24L01_REG_DEFAULT_VAL_DYNPD*/
#define NRF24L01_DPL_P0			0 //Enable dyn. payload length data pipe 0. Requires EN_DPL and ENAA_P0
#define NRF24L01_DPL_P1			1
#define NRF24L01_DPL_P2			2
#define NRF24L01_DPL_P3			3
#define NRF24L01_DPL_P4			4
#define NRF24L01_DPL_P5			5

/*Feature register bits - NRF24L01_REG_FEATURE */
#define NRF24L01_FT_EN_DPL		2	//Enables Dynamic Payload Length
#define NRF24L01_FT_EN_ACK_PAY	1	//Enables Payload with ACK
#define NRF24L01_FT_EN_DYN_ACK	0	//Enables the W_TX_PAYLOAD_NOACK command

//OTHER INFORMATION

/* Transmitter power*/
#define NRF24L01_M18DBM			0 //-18 dBm
#define NRF24L01_M12DBM			1 //-12 dBm
#define NRF24L01_M6DBM			2 //-6 dBm
#define NRF24L01_0DBM			3 //0 dBm

/* Data rates */
#define NRF24L01_2MBPS			0
#define NRF24L01_1MBPS			1
#define NRF24L01_250KBPS		2

/* Configuration */
#define NRF24L01_CONFIG			((1 << NRF24L01_EN_CRC) | (0 << NRF24L01_CRCO))

/***********************Instruction Mnemonics - COMMANDS***********************************************************/
#define NRF24L01_REGISTER_MASK				0x1F

//Last 5 bits will indicate reg. address
#define NRF24L01_READ_REGISTER_MASK(reg)	(0x00 | (NRF24L01_REGISTER_MASK & reg)) 	/*Read command and status registers*/
#define NRF24L01_WRITE_REGISTER_MASK(reg)	(0x20 | (NRF24L01_REGISTER_MASK & reg)) 	/*Write command and status registers. Executable in power down or standby modes only*/
#define NRF24L01_R_RX_PAYLOAD_MASK			0x61  			/*Read RX payload. Payload is deleted from FIFO after it is read. Used in RX mode*/
#define NRF24L01_W_TX_PAYLOAD_MASK			0xA0			/*Write TX payload. A write operation always starts at bye 0 used in TX payload*/
#define NRF24L01_FLUSH_TX_MASK				0xE1			/*Flush TX FIFO. Used in TX mode*/
#define NRF24L01_FLUSH_RX_MASK				0xE2			/*Flush RX FIFO. Used in RX mode. Should not be used during a transmission of acknokledge*/
#define NRF24L01_REUSE_TX_PL_MASK			0xE3			/*Used for a PTX device. Reuse last transmitted payload. (not used)*/
#define NRF24L01_ACTIVATE_MASK				0x50 			/*This write command followed by 0x73 activates: R_RX_PL_WID, W_ACK_PAYLOAD, W_TX_PAYLOAD_NOACK. This is executable in power down or stand by modes only*/
#define NRF24L01_R_RX_PL_WID_MASK			0x60			/*Read RX payload width for the top R_RX_PAYLOAD in the RX FIFO.*/
#define NRF24L01_W_ACK_PAYLOAD_MASK 		0xA0 /*&=PPP*/	/*Used in RX mode. Write payload to be transmitted together with ACK packet*/
#define NRF24L01_W_TX_PAYLOAD_NO_ACK		0xB0			/*Used in TX mode. Disables AUTOACK on this specific packet. (not used)*/
#define NRF24L01_NOP_MASK					0xFF	

/* Flush FIFOs */
#define NRF24L01_FLUSH_TX					do { NRF24L01_CSN_LOW; TM_SPI_Send(NRF24L01_SPI, NRF24L01_FLUSH_TX_MASK); NRF24L01_CSN_HIGH; } while (0)
#define NRF24L01_FLUSH_RX					do { NRF24L01_CSN_LOW; TM_SPI_Send(NRF24L01_SPI, NRF24L01_FLUSH_RX_MASK); NRF24L01_CSN_HIGH; } while (0)

#define NRF24L01_TRANSMISSON_OK 			0
#define NRF24L01_MESSAGE_LOST   			1

#define NRF24L01_CHECK_BIT(reg, bit)       (reg & (1 << bit))

#define ACK_PAY_EN

typedef struct {
	uint8_t PayloadSize;				//Payload size
	uint8_t Channel;					//Channel selected
	TM_NRF24L01_OutputPower_t OutPwr;	//Output power
	TM_NRF24L01_DataRate_t DataRate;	//Data rate
} TM_NRF24L01_t;

/* Private functions */
void TM_NRF24L01_InitPins(void);
void TM_NRF24L01_WriteBit(uint8_t reg, uint8_t bit, uint8_t value);
uint8_t TM_NRF24L01_ReadBit(uint8_t reg, uint8_t bit);
uint8_t TM_NRF24L01_ReadRegister(uint8_t reg);
void TM_NRF24L01_ReadRegisterMulti(uint8_t reg, uint8_t* data, uint8_t count);
void TM_NRF24L01_WriteRegisterMulti(uint8_t reg, uint8_t *data, uint8_t count);
void TM_NRF24L01_SoftwareReset(void);

/* NRF structure */
static TM_NRF24L01_t TM_NRF24L01_Struct;

void TM_NRF24L01_InitPins(void) {
	/* Init pins */
	/* CNS pin */
	TM_GPIO_Init(NRF24L01_CSN_PORT, NRF24L01_CSN_PIN, TM_GPIO_Mode_OUT, TM_GPIO_OType_PP, TM_GPIO_PuPd_UP, TM_GPIO_Speed_Low);
	
	/* CE pin */
	TM_GPIO_Init(NRF24L01_CE_PORT, NRF24L01_CE_PIN, TM_GPIO_Mode_OUT, TM_GPIO_OType_PP, TM_GPIO_PuPd_UP, TM_GPIO_Speed_Low);
	
	/* CSN high = disable SPI */
	NRF24L01_CSN_HIGH;
	
	/* CE low = disable TX/RX */
	NRF24L01_CE_LOW;
}

uint8_t TM_NRF24L01_Init(uint8_t channel, uint8_t payload_size) {
	/* Initialize CE and CSN pins */
	TM_NRF24L01_InitPins();
	
	/* Initialize SPI */
	TM_SPI_Init(NRF24L01_SPI, NRF24L01_SPI_PINS);
	
	/* Max payload is 32bytes */
	if (payload_size > 32) {
		payload_size = 32;
	}
	
	/* Fill structure */
	TM_NRF24L01_Struct.Channel = !channel; /* Set channel to some different value for TM_NRF24L01_SetChannel() function */
	TM_NRF24L01_Struct.PayloadSize = payload_size;
	TM_NRF24L01_Struct.OutPwr = TM_NRF24L01_OutputPower_0dBm;
	TM_NRF24L01_Struct.DataRate = TM_NRF24L01_DataRate_2M;
	
	/* Reset nRF24L01+ to power on registers values */
	TM_NRF24L01_SoftwareReset();
	
	/* Channel select */
	TM_NRF24L01_SetChannel(channel);
	
	
	/* Set RF settings (2mbps, output power) */
	TM_NRF24L01_SetRF(TM_NRF24L01_Struct.DataRate, TM_NRF24L01_Struct.OutPwr);
	
	/* Config register */
	TM_NRF24L01_WriteRegister(NRF24L01_REG_CONFIG, NRF24L01_CONFIG);
	
	/* Enable auto-acknowledgment for all pipes */
	TM_NRF24L01_WriteRegister(NRF24L01_REG_EN_AA, 0x3F);
	
	/* Enable RX addresses */
	TM_NRF24L01_WriteRegister(NRF24L01_REG_EN_RXADDR, 0x3F);

	/* Auto retransmit delay: 1000 (4x250) us and Up to 15 retransmit trials */
	TM_NRF24L01_WriteRegister(NRF24L01_REG_SETUP_RETR, 0x4F);

	/*An ACK packet can contain an optional payload from PRX to PTX. In order to use this feature,
	 the dynamic payload lenght feature must be enabled. The MCU on the PRX side has to upload the 
	 payload by clocking it into the TX FIFO by using the W_ACK_PAYLOAD command. The payload is 
	 pending in the TX FIFO (PRX) until a new packet is received from the PTX. The nRF can have 3 ACK 
	 packet payloads pending in the TX FIFO (PRX) at the same time.*/

	#ifdef ACK_PAY_EN
	/*Activates R_RX_PL_WID, W_ACK_PAYLOAD and W_TX_PAYLOAD_NOACK registers by  ACTIVATE command*/
	TM_NRF24L01_WriteRegister(NRF24L01_ACTIVATE_MASK, 0x73);

	/* Enable Dynamic Payload Length, Payload with ACK, and W_TX_PAYLOAD_NOACK commands */
	TM_NRF24L01_WriteRegister(NRF24L01_REG_FEATURE, (1 << NRF24L01_FT_EN_DPL) | (1 << NRF24L01_FT_EN_ACK_PAY) | (0 << NRF24L01_FT_EN_DYN_ACK));
	
	/* Enable dynamic payload length for all pipes*/
	TM_NRF24L01_WriteRegister(NRF24L01_REG_DYNPD, (1 << NRF24L01_DPL_P0) | (1 << NRF24L01_DPL_P1) | (1 << NRF24L01_DPL_P2) | (1 << NRF24L01_DPL_P3) | (1 << NRF24L01_DPL_P4) | (1 << NRF24L01_DPL_P5));
	
	 /* Enabled RX Addresses for all pipes (used to receive ACK payload in PRX)*/
	TM_NRF24L01_WriteRegister(NRF24L01_REG_EN_RXADDR, (1 << NRF24L01_ERX_P0) | (1 << NRF24L01_ERX_P1) | (1 << NRF24L01_ERX_P2) | (1 << NRF24L01_ERX_P3) | (1 << NRF24L01_ERX_P4) | (1 << NRF24L01_ERX_P5));
	#else

	/* Set pipeline to max possible 32 bytes */
	TM_NRF24L01_WriteRegister(NRF24L01_REG_RX_PW_P0, TM_NRF24L01_Struct.PayloadSize); // Auto-ACK pipe
	TM_NRF24L01_WriteRegister(NRF24L01_REG_RX_PW_P1, TM_NRF24L01_Struct.PayloadSize); // Data payload pipe
	TM_NRF24L01_WriteRegister(NRF24L01_REG_RX_PW_P2, TM_NRF24L01_Struct.PayloadSize);
	TM_NRF24L01_WriteRegister(NRF24L01_REG_RX_PW_P3, TM_NRF24L01_Struct.PayloadSize);
	TM_NRF24L01_WriteRegister(NRF24L01_REG_RX_PW_P4, TM_NRF24L01_Struct.PayloadSize);
	TM_NRF24L01_WriteRegister(NRF24L01_REG_RX_PW_P5, TM_NRF24L01_Struct.PayloadSize);
	
	/* Dynamic length configurations: No dynamic length */
	TM_NRF24L01_WriteRegister(NRF24L01_REG_DYNPD, (0 << NRF24L01_DPL_P0) | (0 << NRF24L01_DPL_P1) | (0 << NRF24L01_DPL_P2) | (0 << NRF24L01_DPL_P3) | (0 << NRF24L01_DPL_P4) | (0 << NRF24L01_DPL_P5));
	#endif
	 
	/* Clear FIFOs */
	NRF24L01_FLUSH_TX;
	NRF24L01_FLUSH_RX;
	
	/* Clear interrupts */
	TM_NRF24L01_Clear_Interrupts();
	
	/* Go to RX mode */
	TM_NRF24L01_PowerUpRx();
	
	/* Return OK */
	return 1;
}

void TM_NRF24L01_SetMyAddress(uint8_t *adr) {
	NRF24L01_CE_LOW;
	TM_NRF24L01_WriteRegisterMulti(NRF24L01_REG_RX_ADDR_P1, adr, 5);
	NRF24L01_CE_HIGH;
}

void TM_NRF24L01_SetTxAddress(uint8_t *adr) {
    NRF24L01_CE_LOW;
	TM_NRF24L01_WriteRegisterMulti(NRF24L01_REG_RX_ADDR_P0, adr, 5);
	TM_NRF24L01_WriteRegisterMulti(NRF24L01_REG_TX_ADDR, adr, 5);
    NRF24L01_CE_HIGH;
}

void TM_NRF24L01_WriteBit(uint8_t reg, uint8_t bit, uint8_t value) {
	uint8_t tmp;
	/* Read register */
	tmp = TM_NRF24L01_ReadRegister(reg);
	/* Make operation */
	if (value) {
		tmp |= 1 << bit;
	} else {
		tmp &= ~(1 << bit);
	}
	/* Write back */
	TM_NRF24L01_WriteRegister(reg, tmp);
}

uint8_t TM_NRF24L01_ReadBit(uint8_t reg, uint8_t bit) {
	uint8_t tmp;
	tmp = TM_NRF24L01_ReadRegister(reg);
	if (!NRF24L01_CHECK_BIT(tmp, bit)) {
		return 0;
	}
	return 1;
}

uint8_t TM_NRF24L01_ReadRegister(uint8_t reg) {
	uint8_t value;
	NRF24L01_CSN_LOW;
	TM_SPI_Send(NRF24L01_SPI, NRF24L01_READ_REGISTER_MASK(reg));
	value = TM_SPI_Send(NRF24L01_SPI, NRF24L01_NOP_MASK);
	NRF24L01_CSN_HIGH;
	
	return value;
}

void TM_NRF24L01_ReadRegisterMulti(uint8_t reg, uint8_t* data, uint8_t count) {
	NRF24L01_CSN_LOW;
	TM_SPI_Send(NRF24L01_SPI, NRF24L01_READ_REGISTER_MASK(reg));
	TM_SPI_ReadMulti(NRF24L01_SPI, data, NRF24L01_NOP_MASK, count);
	NRF24L01_CSN_HIGH;
}

void TM_NRF24L01_WriteRegister(uint8_t reg, uint8_t value) {
	NRF24L01_CSN_LOW;
	TM_SPI_Send(NRF24L01_SPI, NRF24L01_WRITE_REGISTER_MASK(reg));
	TM_SPI_Send(NRF24L01_SPI, value);
	NRF24L01_CSN_HIGH;
}

void TM_NRF24L01_WriteRegisterMulti(uint8_t reg, uint8_t *data, uint8_t count) {
	NRF24L01_CSN_LOW;
	TM_SPI_Send(NRF24L01_SPI, NRF24L01_WRITE_REGISTER_MASK(reg));
	TM_SPI_WriteMulti(NRF24L01_SPI, data, count);
	NRF24L01_CSN_HIGH;
}

void TM_NRF24L01_PowerUpTx(void) {
	TM_NRF24L01_Clear_Interrupts();
	TM_NRF24L01_WriteRegister(NRF24L01_REG_CONFIG, NRF24L01_CONFIG | (0 << NRF24L01_PRIM_RX) | (1 << NRF24L01_PWR_UP));
}

void TM_NRF24L01_PowerUpRx(void) {
	/* Disable RX/TX mode */
	NRF24L01_CE_LOW;
	/* Clear RX buffer */
	NRF24L01_FLUSH_RX;
	/* Clear interrupts */
	TM_NRF24L01_Clear_Interrupts();
	/* Setup RX mode */
	TM_NRF24L01_WriteRegister(NRF24L01_REG_CONFIG, NRF24L01_CONFIG | 1 << NRF24L01_PWR_UP | 1 << NRF24L01_PRIM_RX);
	/* Start listening */
	NRF24L01_CE_HIGH;
}

void TM_NRF24L01_PowerDown(void) {
	NRF24L01_CE_LOW;
	TM_NRF24L01_WriteBit(NRF24L01_REG_CONFIG, NRF24L01_PWR_UP, 0);
}

//Funcion que transmite datos estaticos de TX a RX
/* Write TX-payload: 1 ? 32 bytes. A write operation
 * always starts at byte 0 used in TX payload.*/

 //revisar_NAK quitar los printf
void TM_NRF24L01_Transmit(uint8_t *data, uint8_t count) {
    #ifndef ACK_PAY_EN
	count = TM_NRF24L01_Struct.PayloadSize;
    #endif

	/* Chip enable put to low, disable it */
	NRF24L01_CE_LOW;
	
	/* Go to power up tx mode */
	TM_NRF24L01_PowerUpTx();
    
	/* Clear TX FIFO from NRF24L01+ */
	NRF24L01_FLUSH_TX;
    
	/* Send payload to nRF24L01+ */
	NRF24L01_CSN_LOW;
    
	/* Send write payload command */
	TM_SPI_Send(NRF24L01_SPI, NRF24L01_W_TX_PAYLOAD_MASK);
    
	/* Fill payload with data*/
	TM_SPI_WriteMulti(NRF24L01_SPI, data, count);
    
	/* Disable SPI */
	NRF24L01_CSN_HIGH;
	
	/* Send data! */
	NRF24L01_CE_HIGH;
}

/* Used in RX mode.
 * Write Payload to be transmitted together with ACK packet on PIPE PPP. (PPP valid in the range from 000 to 101).
 * Maximum three ACK packet payloads can be pending. Payloads with same PPP are handled using first in - first out principle. 
 * Write payload: 1? 32 bytes. A write operation always starts at byte 0.*/

//Funcion que VACIA la cola de TX FIFO (en PRX) y aniade un PAYLOAD.
void TM_NRF24L01_WriteAckPayload(uint8_t pipe, uint8_t* data, uint8_t length) {
	/* Clear TX FIFO from NRF24L01+ */
	NRF24L01_FLUSH_TX;
    
	printf("After flush TX FIFO: 0x%02X\n", TM_NRF24L01_TxFifoEmpty());
	
	/* Send payload to nRF24L01+ */
	NRF24L01_CSN_LOW;
	
	/* Send write payload command */
	TM_SPI_Send(NRF24L01_SPI, NRF24L01_W_ACK_PAYLOAD_MASK | (pipe & 0x07));

	/* Fill payload with data*/
	TM_SPI_WriteMulti(NRF24L01_SPI, data, length);

	/* Disable SPI */
	NRF24L01_CSN_HIGH;
}

/* Read RX-payload: 1 ? 32 bytes. A read operation
 * always starts at byte 0. Payload is deleted from
 * FIFO after it is read. Used in RX mode.*/

//Funcion que lee los datos recibidos. Se llama en modo RX. Vacia la cola RX tras leerlos.
//Tanto para leer el payload normal como el del ACK.
//Se tienen que mandar datos "basura" para que el sensor responda con los datos reales

void TM_NRF24L01_GetData(uint8_t* data) {
	/* Pull down chip select */
	NRF24L01_CSN_LOW;
	/* Send read payload command*/
	TM_SPI_Send(NRF24L01_SPI, NRF24L01_R_RX_PAYLOAD_MASK);
	/* Read payload */
	TM_SPI_SendMulti(NRF24L01_SPI, data, data, TM_NRF24L01_Struct.PayloadSize);
	/* Pull up chip select */
	NRF24L01_CSN_HIGH;
	
	/* Reset status register, clear RX_DR interrupt flag */
	TM_NRF24L01_WriteRegister(NRF24L01_REG_STATUS, (1 << NRF24L01_RX_DR));
}

//Funcion que mira si hay datos disponibles para leer en la RX FIFO (Interrupt Data Ready).
uint8_t TM_NRF24L01_DataReady(void) {
	uint8_t status = TM_NRF24L01_GetStatus();
	
	if (NRF24L01_CHECK_BIT(status, NRF24L01_RX_DR)) {
		return 1;
	}
	return !TM_NRF24L01_RxFifoEmpty();
}

//Funcion que mira si la RX FIFO esta vacia o no
//0: not empty
//1: empty

uint8_t TM_NRF24L01_RxFifoEmpty(void) {
	uint8_t reg = TM_NRF24L01_ReadRegister(NRF24L01_REG_FIFO_STATUS);
	return NRF24L01_CHECK_BIT(reg, NRF24L01_RX_EMPTY);
}

//Funcion que mira si la RX FIFO esta vacia o no
//0: not empty
//1: empty
uint8_t TM_NRF24L01_TxFifoEmpty(void) {
	uint8_t reg = TM_NRF24L01_ReadRegister(NRF24L01_REG_FIFO_STATUS);
	return NRF24L01_CHECK_BIT(reg, NRF24L01_TX_EMPTY);
}

//Funcion que devuelve el registro status del sensor.
uint8_t TM_NRF24L01_GetStatus(void) {
	uint8_t status;
	
	NRF24L01_CSN_LOW;
	/* First received byte is always status register */
	status = TM_SPI_Send(NRF24L01_SPI, NRF24L01_NOP_MASK);
	/* Pull up chip select */
	NRF24L01_CSN_HIGH;
	
	return status;
}

TM_NRF24L01_Transmit_Status_t TM_NRF24L01_GetTransmissionStatus(void) {
	uint8_t status = TM_NRF24L01_GetStatus();
	if (NRF24L01_CHECK_BIT(status, NRF24L01_TX_DS)) {
		/* Successfully sent */
		return TM_NRF24L01_Transmit_Status_Ok;
	} else if (NRF24L01_CHECK_BIT(status, NRF24L01_MAX_RT)) {
		/* Message lost */
		return TM_NRF24L01_Transmit_Status_Lost;
	}
	
	/* Still sending */
	return TM_NRF24L01_Transmit_Status_Sending;
}

void TM_NRF24L01_SoftwareReset(void) {
	uint8_t data[5];
	
	TM_NRF24L01_WriteRegister(NRF24L01_REG_CONFIG, 		NRF24L01_REG_DEFAULT_VAL_CONFIG);
	TM_NRF24L01_WriteRegister(NRF24L01_REG_EN_AA,		NRF24L01_REG_DEFAULT_VAL_EN_AA);
	TM_NRF24L01_WriteRegister(NRF24L01_REG_EN_RXADDR, 	NRF24L01_REG_DEFAULT_VAL_EN_RXADDR);
	TM_NRF24L01_WriteRegister(NRF24L01_REG_SETUP_AW, 	NRF24L01_REG_DEFAULT_VAL_SETUP_AW);
	TM_NRF24L01_WriteRegister(NRF24L01_REG_SETUP_RETR, 	NRF24L01_REG_DEFAULT_VAL_SETUP_RETR);
	TM_NRF24L01_WriteRegister(NRF24L01_REG_RF_CH, 		NRF24L01_REG_DEFAULT_VAL_RF_CH);
	TM_NRF24L01_WriteRegister(NRF24L01_REG_RF_SETUP, 	NRF24L01_REG_DEFAULT_VAL_RF_SETUP);
	TM_NRF24L01_WriteRegister(NRF24L01_REG_STATUS, 		NRF24L01_REG_DEFAULT_VAL_STATUS);
	TM_NRF24L01_WriteRegister(NRF24L01_REG_OBSERVE_TX, 	NRF24L01_REG_DEFAULT_VAL_OBSERVE_TX);
	TM_NRF24L01_WriteRegister(NRF24L01_REG_RPD, 		NRF24L01_REG_DEFAULT_VAL_RPD);
	
	//P0
	data[0] = NRF24L01_REG_DEFAULT_VAL_RX_ADDR_P0_0;
	data[1] = NRF24L01_REG_DEFAULT_VAL_RX_ADDR_P0_1;
	data[2] = NRF24L01_REG_DEFAULT_VAL_RX_ADDR_P0_2;
	data[3] = NRF24L01_REG_DEFAULT_VAL_RX_ADDR_P0_3;
	data[4] = NRF24L01_REG_DEFAULT_VAL_RX_ADDR_P0_4;
	TM_NRF24L01_WriteRegisterMulti(NRF24L01_REG_RX_ADDR_P0, data, 5);
	
	//P1
	data[0] = NRF24L01_REG_DEFAULT_VAL_RX_ADDR_P1_0;
	data[1] = NRF24L01_REG_DEFAULT_VAL_RX_ADDR_P1_1;
	data[2] = NRF24L01_REG_DEFAULT_VAL_RX_ADDR_P1_2;
	data[3] = NRF24L01_REG_DEFAULT_VAL_RX_ADDR_P1_3;
	data[4] = NRF24L01_REG_DEFAULT_VAL_RX_ADDR_P1_4;
	TM_NRF24L01_WriteRegisterMulti(NRF24L01_REG_RX_ADDR_P1, data, 5);
	
	TM_NRF24L01_WriteRegister(NRF24L01_REG_RX_ADDR_P2, 	NRF24L01_REG_DEFAULT_VAL_RX_ADDR_P2);
	TM_NRF24L01_WriteRegister(NRF24L01_REG_RX_ADDR_P3, 	NRF24L01_REG_DEFAULT_VAL_RX_ADDR_P3);
	TM_NRF24L01_WriteRegister(NRF24L01_REG_RX_ADDR_P4, 	NRF24L01_REG_DEFAULT_VAL_RX_ADDR_P4);
	TM_NRF24L01_WriteRegister(NRF24L01_REG_RX_ADDR_P5, 	NRF24L01_REG_DEFAULT_VAL_RX_ADDR_P5);
	
	//TX
	data[0] = NRF24L01_REG_DEFAULT_VAL_TX_ADDR_0;
	data[1] = NRF24L01_REG_DEFAULT_VAL_TX_ADDR_1;
	data[2] = NRF24L01_REG_DEFAULT_VAL_TX_ADDR_2;
	data[3] = NRF24L01_REG_DEFAULT_VAL_TX_ADDR_3;
	data[4] = NRF24L01_REG_DEFAULT_VAL_TX_ADDR_4;
	TM_NRF24L01_WriteRegisterMulti(NRF24L01_REG_TX_ADDR, data, 5);
	
	TM_NRF24L01_WriteRegister(NRF24L01_REG_RX_PW_P0, 	NRF24L01_REG_DEFAULT_VAL_RX_PW_P0);
	TM_NRF24L01_WriteRegister(NRF24L01_REG_RX_PW_P1, 	NRF24L01_REG_DEFAULT_VAL_RX_PW_P1);
	TM_NRF24L01_WriteRegister(NRF24L01_REG_RX_PW_P2, 	NRF24L01_REG_DEFAULT_VAL_RX_PW_P2);
	TM_NRF24L01_WriteRegister(NRF24L01_REG_RX_PW_P3, 	NRF24L01_REG_DEFAULT_VAL_RX_PW_P3);
	TM_NRF24L01_WriteRegister(NRF24L01_REG_RX_PW_P4, 	NRF24L01_REG_DEFAULT_VAL_RX_PW_P4);
	TM_NRF24L01_WriteRegister(NRF24L01_REG_RX_PW_P5, 	NRF24L01_REG_DEFAULT_VAL_RX_PW_P5);
	TM_NRF24L01_WriteRegister(NRF24L01_REG_FIFO_STATUS, NRF24L01_REG_DEFAULT_VAL_FIFO_STATUS);
	TM_NRF24L01_WriteRegister(NRF24L01_REG_DYNPD, 		NRF24L01_REG_DEFAULT_VAL_DYNPD);
	TM_NRF24L01_WriteRegister(NRF24L01_REG_FEATURE, 	NRF24L01_REG_DEFAULT_VAL_FEATURE);
}

//Funcion que mira cuantas veces se han perdido o retransmitido los paquetes.
uint8_t TM_NRF24L01_GetRetransmissionsCount(void) {
	/* Low 4 bits */
	return TM_NRF24L01_ReadRegister(NRF24L01_REG_OBSERVE_TX) & 0x0F;
}

void TM_NRF24L01_SetChannel(uint8_t channel) {
	if (channel <= 125 && channel != TM_NRF24L01_Struct.Channel) {
		/* Store new channel setting */
		TM_NRF24L01_Struct.Channel = channel;
		/* Write channel */
		TM_NRF24L01_WriteRegister(NRF24L01_REG_RF_CH, channel);
	}
}

void TM_NRF24L01_SetRF(TM_NRF24L01_DataRate_t DataRate, TM_NRF24L01_OutputPower_t OutPwr) {
	uint8_t tmp = 0;
	TM_NRF24L01_Struct.DataRate = DataRate;
	TM_NRF24L01_Struct.OutPwr = OutPwr;
	
	if (DataRate == TM_NRF24L01_DataRate_2M) {
		tmp |= 1 << NRF24L01_RF_DR_HIGH;
	} else if (DataRate == TM_NRF24L01_DataRate_250k) {
		tmp |= 1 << NRF24L01_RF_DR_LOW;
	}
	/* If 1Mbps, all bits set to 0 */
	
	if (OutPwr == TM_NRF24L01_OutputPower_0dBm) {
		tmp |= 3 << NRF24L01_RF_PWR;
	} else if (OutPwr == TM_NRF24L01_OutputPower_M6dBm) {
		tmp |= 2 << NRF24L01_RF_PWR;
	} else if (OutPwr == TM_NRF24L01_OutputPower_M12dBm) {
		tmp |= 1 << NRF24L01_RF_PWR;
	}
	
	TM_NRF24L01_WriteRegister(NRF24L01_REG_RF_SETUP, tmp);
}

//Funcion que devuelve el estado del registro status del sensor (principalmente orientado a IRQs)
uint8_t TM_NRF24L01_Read_Interrupts(TM_NRF24L01_IRQ_t* IRQ) {
	IRQ->Status = TM_NRF24L01_GetStatus();
	return IRQ->Status;
}

void TM_NRF24L01_Clear_Interrupts(void) {
	TM_NRF24L01_WriteRegister(0x07, 0x70);
}

