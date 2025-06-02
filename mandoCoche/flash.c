#include "flash.h"
#include "Driver_SPI.h"
#include "app_main.h"
/*
Flash memory is a device that operates via SPI.
D1 - input, connected to our MOSI.
D0 - output, connected to our MISO.

It is a 16Mbit memory, consisting of 32 blocks of 64KBytes each.
Each block has 16 sectors of 4KBytes. Each sector is composed of 16 pages, each page being 256 bytes.
Thus, the total number of pages is: 32 * 16 * 16 = 8192 pages, with a total of 2 million bytes.

It is a NOR memory, meaning that in order to write, the memory needs to be reset to FF.
Before writing, the memory must be erased. The minimum space to erase is 1 sector (4KBytes).

Therefore, we will use a function called W25Q16_Write, which saves the previous information, updates it,
erases the entire sector memory, and then rewrites the information.
*/

//flags and hilos
#define QUEUE_MAX						10                //maximum number of objects in a queue
//memory consts
#define nBlock							32								//32 blocks in our 16MBytes memory
#define PAGES_FOR_SECTOR				16								//defines the number of pages in one sector

//Data application
#define MAX_CONSUMPTION_DATA			10			//num max of consumption data information in flash
#define CONSUMPTION_SIZE_B				32			//32 bits (FLOAT)
#define HORA_SIZE_B						24			//3 uint8_t (hora + minutos + segundos)

typedef union {
    float f;
    uint8_t bytes[4];
} FloatUnion;

//SPI init
extern ARM_DRIVER_SPI Driver_SPI4;
static ARM_DRIVER_SPI* SPIdrv = &Driver_SPI4;

//thread
static osThreadId_t tid_flash;															//flash thread id used in main thread
osMessageQueueId_t id_flash_commands_queue;
static void Th_flash (void *argument);

//LOCAL FUNCTIONS

//initialize
static void W25Q16_Init_SPI(void);
static void SPI_callback(uint32_t event);
static void W25Q16_Init (void);
//recurrent functions
static void W25Q16_WriteInstruction(uint8_t val);
static void W25Q16_Reset(void);
static void W25Q16_Erase_64kBlock (uint16_t numBlock);
static void W25Q16_Erase_Sector (uint16_t numSector);
static uint16_t W25Q16_ReadID(uint8_t number_id);
static void W25Q16_Read (uint32_t startPage, uint8_t offset, uint32_t size, uint8_t *rData);
static void W25Q16_FastRead (uint32_t startPage, uint8_t offset, uint32_t size, uint8_t *rData);
static void W25Q16_WritePage_Clean (uint32_t page, uint16_t offset, uint32_t size, uint8_t *data);
//static void W25Q16_Write (uint32_t page, uint16_t offset, uint32_t size, uint8_t *data);
static void write_enable (void);
static void write_disable (void);
//static uint32_t bytesToModify (int32_t size, uint16_t offset);
static uint32_t bytesToWrite (uint32_t size, uint32_t offset);
static void erase_usable_memory (void);

//power up and down
void W25Q16_PowerDown (void);
void W25Q16_PowerUp (void);

static void addConsumption(uint8_t position, float *consumption, char* hora);
static void getAllConsumptions(float *consumptions);
static void floatToBytes(float valor, uint8_t bytes[sizeof(float)]);
static float bytesToFloat(uint8_t bytes[sizeof(float)]);

//tests
static void leer_mem_entera(void) ;
static void test_write_read(void);
//Variables externas

//Init flash
void Init_FlashControl (void) {
  //const static osThreadAttr_t th_attr = {.stack_size = 7000};
	id_flash_commands_queue = osMessageQueueNew(QUEUE_MAX, sizeof(MSGQUEUE_FLASH_t), NULL);
	tid_flash = osThreadNew(Th_flash, NULL, NULL); 
}

static void Th_flash (void *argument) {
  MSGQUEUE_FLASH_t flash_msg_rec;
  uint8_t posicion_consumo = 0;

  W25Q16_Init();
  erase_usable_memory();

//Funciones de test
  //leer_mem_entera();
  //test_write_read();
	
  while (1) {
		if (osMessageQueueGet(id_flash_commands_queue, &flash_msg_rec, NULL, osWaitForever)==osOK) {
			switch (flash_msg_rec.command) 
			{
				case FLASH_CMD__ADD_CONSUMPTION:
				//Se añade hasta un maximo de 10 consumos de manera circular
                    addConsumption(posicion_consumo, flash_msg_rec.consumption, flash_msg_rec.hora);
                    posicion_consumo = (posicion_consumo == (MAX_CONSUMPTION_DATA - 1)) ? 0 : (posicion_consumo + 1);
				break;
				case FLASH_CMD__GET_ALL_CONSUMPTION:
				//Se le pasa un puntero de consumos donde se guardaran todos
                    getAllConsumptions(flash_msg_rec.consumption);
                    osThreadFlagsSet(id_thread__app_main, FLAG__CONSUMO_READY_FLASH);
				break;
				case FLASH_CMD__ERASE:
                    erase_usable_memory();
				break;
			}
		}
	}
}


//function that erase all the memory we will use to store user, event and password information
static void erase_usable_memory (void) {
	for (int i = 0; i < 3; i++) {
		W25Q16_Erase_64kBlock(i);
	}
}

//depuration code to read all memory data from sector 0
static void leer_mem_entera(void) {
	uint8_t previousData[50];
	W25Q16_FastRead(0, 0, 50, previousData);
	previousData[0] = 0x00;
}

//depuration test to see if the memory can write and read some data
static void test_write_read (void) {
	uint8_t num = 1;
	uint8_t TxData[32] =  {num};
  	uint8_t RxData[32];
	while (1) {
		for (int e = 0; e < 1000; e++) 
		{
       		num++;
      		TxData[0] = num;
      		W25Q16_WritePage_Clean(0, num, 32,TxData);	//Write byte to byte 1 add number
      		W25Q16_FastRead (0, 0, sizeof(RxData), RxData);	//Read all sector
		}
	}
}

//Init SPI for W25Q16 memory
static void W25Q16_Init_SPI(void){
  __HAL_RCC_GPIOE_CLK_ENABLE();
  
  static GPIO_InitTypeDef GPIO_InitStruct_RFID;
  /*CS*/    //SPI_CS -- SPI_B_NSS       PE11
  __HAL_RCC_GPIOA_CLK_ENABLE();
  GPIO_InitStruct_RFID.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct_RFID.Pull = GPIO_PULLUP;
  GPIO_InitStruct_RFID.Speed = GPIO_SPEED_FREQ_HIGH;
  GPIO_InitStruct_RFID.Pin = GPIO_PIN_11;
  HAL_GPIO_Init(GPIOE, &GPIO_InitStruct_RFID);
  HAL_GPIO_WritePin(GPIOE, GPIO_PIN_11, GPIO_PIN_SET);
  
  /*SPI*/   
  SPIdrv->Initialize(SPI_callback);
  SPIdrv-> PowerControl(ARM_POWER_FULL);
  SPIdrv-> Control(ARM_SPI_MODE_MASTER | ARM_SPI_CPOL1_CPHA1 | ARM_SPI_MSB_LSB | ARM_SPI_DATA_BITS (8), 1000000);
}

static void SPI_callback(uint32_t event){
  uint32_t error;
    switch (event) {
    case ARM_SPI_EVENT_TRANSFER_COMPLETE:
        error = osThreadFlagsSet(tid_flash, TRANSFER_COMPLETE);
        if(error == osFlagsErrorUnknown) {
          __breakpoint(0);
        } else if (error == osFlagsErrorParameter) {
          osThreadFlagsSet(tid_flash, TRANSFER_COMPLETE);
          __breakpoint(0);
        } else if (error == osFlagsErrorResource) {
          __breakpoint(0);
        }
        break;
    case ARM_SPI_EVENT_DATA_LOST:
        /*  Occurs in slave mode when data is requested/sent by master
            but send/receive/transfer operation has not been started
            and indicates that data is lost. Occurs also in master mode
            when driver cannot transfer data fast enough. */
        __breakpoint(0);  /* Error: Call debugger or replace with custom error handling */
        break;
    case ARM_SPI_EVENT_MODE_FAULT:
        /*  Occurs in master mode when Slave Select is deactivated and
            indicates Master Mode Fault. */
        __breakpoint(0);  /* Error: Call debugger or replace with custom error handling */
        break;
    }
}

//initialize the driver SPI and pin configuration and reset the device (NOT all data to FF)
static void W25Q16_Init (void) {
	W25Q16_Init_SPI();
	W25Q16_Reset();
}

//write a single instruction (not so useful)
static void W25Q16_WriteInstruction(uint8_t val) {
	//CS low
	HAL_GPIO_WritePin(GPIOE, GPIO_PIN_11, GPIO_PIN_RESET);
	//send address and data
	SPIdrv->Send(&val, 1);
	osThreadFlagsWait(TRANSFER_COMPLETE, osFlagsWaitAny, osWaitForever);
	//CS high
	HAL_GPIO_WritePin(GPIOE, GPIO_PIN_11, GPIO_PIN_SET);
}

//reset the device (NOT all to FF)
static void W25Q16_Reset(void) {
	W25Q16_WriteInstruction(ENABLE_RESET);
	W25Q16_WriteInstruction(RESET_DEVICE);
}

//read the device id
static uint16_t W25Q16_ReadID(uint8_t number_id) {
	uint8_t instuction = READ_ID;
  uint32_t rx_data;
  //CS low
	HAL_GPIO_WritePin(GPIOE, GPIO_PIN_11, GPIO_PIN_RESET);
	//send address and data
	SPIdrv->Send(&instuction, 1);
	osThreadFlagsWait(TRANSFER_COMPLETE, osFlagsWaitAny, osWaitForever);
	SPIdrv->Receive(&rx_data, 3);
  osThreadFlagsWait(TRANSFER_COMPLETE, osFlagsWaitAny, osWaitForever);
  //CS high
	HAL_GPIO_WritePin(GPIOE, GPIO_PIN_11, GPIO_PIN_SET);

  return ((rx_data>>8)&0xFFFF);
}

//you can read the entire memory in a single instruction
//You need to send the read instruction followed by a 24-bit address - memory address
static void W25Q16_Read (uint32_t startPage, uint8_t offset, uint32_t size, uint8_t *rData) {
	uint8_t tData[4];
	uint32_t memAddr = (startPage*256) + offset; //page contains 256 bytes
	
	tData[0] = READ_DATA;	//read enable command (MSB)
	tData [1] = (memAddr>>16)&0xFF;
	tData [2] = (memAddr>>8)&0xFF;
	tData [3] = (memAddr)&0xFF;
  
  //CS low
	HAL_GPIO_WritePin(GPIOE, GPIO_PIN_11, GPIO_PIN_RESET);
	//send address and data
	SPIdrv->Send(tData, 4);
	osThreadFlagsWait(TRANSFER_COMPLETE, osFlagsWaitAny, osWaitForever);
	SPIdrv->Receive(rData, size);
  osThreadFlagsWait(TRANSFER_COMPLETE, osFlagsWaitAny, osWaitForever);

  //CS high
	HAL_GPIO_WritePin(GPIOE, GPIO_PIN_11, GPIO_PIN_SET);
}

//Fast Read instruction operate at the highest possible frequency (Fr = 50Mhz)
//we need to add 8 dummy clocks after the 24 bits address
static void W25Q16_FastRead (uint32_t startPage, uint8_t offset, uint32_t size, uint8_t *rData) {
	uint8_t tData[5];
	uint32_t memAddr = (startPage*256) + offset;  //page is 256 bytes
	
	tData[0] 	=	READ_FAST;	//read fast enable command
	tData [1] =	(memAddr>>16)&0xFF;
	tData [2] =	(memAddr>>8)&0xFF;
	tData [3] =	(memAddr)&0xFF;
	tData[4] 	= 0;	//dummy clocks
  
  //CS low
	HAL_GPIO_WritePin(GPIOE, GPIO_PIN_11, GPIO_PIN_RESET);
	//send address and data
	SPIdrv->Send(tData, 5);
	osThreadFlagsWait(TRANSFER_COMPLETE, osFlagsWaitAny, osWaitForever);
	SPIdrv->Receive(rData, size);
  	osThreadFlagsWait(TRANSFER_COMPLETE, osFlagsWaitAny, osWaitForever);

  //CS high
	HAL_GPIO_WritePin(GPIOE, GPIO_PIN_11, GPIO_PIN_SET);
}

//write enable intruction, needed for writing and erasing
static void write_enable (void) {
	W25Q16_WriteInstruction(WRITE_ENABLE);
	osDelay(5);
}

//write disable. Needed after a write and erase instruction
static void write_disable (void) {
	W25Q16_WriteInstruction(WRITE_DISABLE);
	osDelay(5);
}

//the minimum space to erase is a sector (4kBytes)
//before erase you need to execute a write enable instruction
static void W25Q16_Erase_64kBlock (uint16_t numBlock) {
	uint8_t tData[4];
	uint32_t memAddr = numBlock*16*16*256;
	
	write_enable();
	
	tData[0] = ERASE_BLOCK;
	tData [1] =	(memAddr>>16)&0xFF;
	tData [2] =	(memAddr>>8)&0xFF;
	tData [3] =	(memAddr)&0xFF;
	
	//CS low
	HAL_GPIO_WritePin(GPIOE, GPIO_PIN_11, GPIO_PIN_RESET);
	//send address and data
	SPIdrv->Send(tData, 4);
	osThreadFlagsWait(TRANSFER_COMPLETE, osFlagsWaitAny, osWaitForever);
	//CS high
	HAL_GPIO_WritePin(GPIOE, GPIO_PIN_11, GPIO_PIN_SET);
	
	osDelay(2000); 	//max time for erase block
	
	write_disable();
}

static void W25Q16_Erase_Sector (uint16_t numSector) {
	uint8_t tData[4];
	uint32_t memAddr = numSector*16*256;
	
	write_enable();
	
	tData[0] = ERASE_SECTOR;
	tData [1] =	(memAddr>>16)&0xFF;
	tData [2] =	(memAddr>>8)&0xFF;
	tData [3] =	(memAddr)&0xFF;
	
	//CS low
	HAL_GPIO_WritePin(GPIOE, GPIO_PIN_11, GPIO_PIN_RESET);
	//send address and data
	SPIdrv->Send(tData, 4);
	osThreadFlagsWait(TRANSFER_COMPLETE, osFlagsWaitAny, osWaitForever);
	//CS high
	HAL_GPIO_WritePin(GPIOE, GPIO_PIN_11, GPIO_PIN_SET);
	
	osDelay(450); 	//max time for erase block
	
	write_disable();
}

static uint32_t bytesToWrite (uint32_t size, uint32_t offset) {
	if ((size+offset) < 256) return size;
	else return 256-offset;
}

//you can write max data of a page, but you need to erase the sector first
//this functions erase the entire sector and then write in some pages
//if you exceed the number of bytes in a page, you will overwrite the data from the beggining
static void W25Q16_WritePage_Clean (uint32_t page, uint16_t offset, uint32_t size, uint8_t *data) {
	//256 bytes to write, 4 bytes for the address and 1 byte for instruction
  uint8_t tData[266];
	uint32_t startPage = page;
	uint32_t endPage = startPage + ((size+offset-1)/256);
	uint32_t numPages = endPage-startPage+1;
	
	uint16_t startSector = startPage/16;
	uint16_t endSector = endPage/16;
	uint16_t numSectors = endSector-startSector+1;
	
	for(uint16_t i = 0; i < numSectors ;i++) {
		W25Q16_Erase_Sector(startSector+i);//antes ponia startSector++
	}
	
	uint32_t dataPosition = 0;
  //write the data
	for (uint32_t i = 0; i<numPages; i++) {
		uint32_t memAddr = (startPage*256)+offset;
		uint16_t bytesRemaining = bytesToWrite(size, offset);
		uint32_t indx = 0;
		
		write_enable();
		
		tData[0] = PAGE_PROGRAM;
		tData [1] =	(memAddr>>16)&0xFF;
		tData [2] =	(memAddr>>8)&0xFF;
		tData [3] =	(memAddr)&0xFF;
	
		indx = 4;
	
		uint16_t bytesToSend = bytesRemaining + indx;
		for (uint16_t i = 0; i < bytesRemaining; i++) {
			tData[indx++] = data[i+dataPosition];
		}
		
		if (bytesToSend > 250) {
			//CS low
       HAL_GPIO_WritePin(GPIOE, GPIO_PIN_11, GPIO_PIN_RESET);
       //send address and data
       SPIdrv->Send(tData, 100);
       osThreadFlagsWait(TRANSFER_COMPLETE, osFlagsWaitAny, osWaitForever);
       SPIdrv->Send(tData+100, bytesToSend-100);
       osThreadFlagsWait(TRANSFER_COMPLETE, osFlagsWaitAny, osWaitForever);
      //CS high
       HAL_GPIO_WritePin(GPIOE, GPIO_PIN_11, GPIO_PIN_SET);
		} else {
			//CS low
       HAL_GPIO_WritePin(GPIOE, GPIO_PIN_11, GPIO_PIN_RESET);
       //send address and data
       SPIdrv->Send(tData, bytesToSend);
       osThreadFlagsWait(TRANSFER_COMPLETE, osFlagsWaitAny, osWaitForever);
       //CS high
       HAL_GPIO_WritePin(GPIOE, GPIO_PIN_11, GPIO_PIN_SET);
		}
		startPage++;
		offset = 0;
		size = size-bytesRemaining;
		dataPosition = dataPosition+bytesRemaining;
		
		//page programm max time 3 ms
		osDelay(5);
		write_disable();
	}
}

static void addConsumption(uint8_t position, float *consumption, char* hora)
{
	uint8_t consumption_data[HORA_SIZE_B + CONSUMPTION_SIZE_B];
	floatToBytes(*consumption, consumption_data);
	
	W25Q16_WritePage_Clean(position*PAGES_FOR_SECTOR, 0, CONSUMPTION_SIZE_B, consumption_data);
}

//Se le pasa un array de MAX_CONSUMPTION_DATA
static void getAllConsumptions(float *consumptions)
{
    uint8_t consumption_data[CONSUMPTION_SIZE_B];

    for (uint8_t position = 0; position < MAX_CONSUMPTION_DATA; position++) 
    {
        // Leer los 4 bytes de la memoria flash
        W25Q16_FastRead(position * PAGES_FOR_SECTOR, 0, CONSUMPTION_SIZE_B, consumption_data);

        // Copiar los 4 bytes en el float correspondiente
        memcpy(&consumptions[position], consumption_data, sizeof(float));
    }
}


static void floatToBytes(float valor, uint8_t bytes[4])
{
    uint8_t *p = (uint8_t*)&valor;
    for (int i = 0; i < 4; i++) {
        bytes[i] = p[i];
    }
}

static float bytesToFloat(uint8_t bytes[4])
{
    FloatUnion fu;
    for (int i = 0; i < 4; i++) {
        fu.bytes[i] = bytes[i];
    }
    return fu.f;
}

static void W25Q16_PowerDown (void) {
  W25Q16_WriteInstruction(POWER_DOWN);
  osDelay (5);
}

static void W25Q16_PowerUp(void) {
  W25Q16_WriteInstruction(POWER_UP);
  osDelay (5);
}


//HACER FUNCION QUE SOLO TE SAQUE 5 USUARIOS POR LAS COLAS


/* POINTER USAGE SUMMARY FOR UNDERSTANDING THE CODE
int *ptr;
int x = 10;

ptr = &x; assigns the memory address of x to the pointer, now the pointer points to where x is located
So if we look at (*ptr), the content of the pointer, it equals 10

*ptr = 20; Assigns the value of 20 to the content of the pointer.
Consequently, now x is worth 20.

*/
