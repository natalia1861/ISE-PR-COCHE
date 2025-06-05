#include "flash.h"
#include "Driver_SPI.h"
#include "app_main.h"
/*
Flash memory is a device that operates via SPI.
D1 - input (MOSI)
D0 - output (MISO)

Caracteristicas:
- Memoria de 16 Mbit = 2 MB
- 32 bloques de 64KB cada uno
- Cada bloque tiene 16 sectores de 4KB
- Cada sector tiene 16 paginas de 256 bytes

IMPORTANTE: la memoria es tipo NOR, por lo tanto para escribir primero hay que borrar (poner a 0xFF).
El borrado minimo posible es por sector (4KB).

Para escribir se usara W25Q16_WritePage_Clean`, que guarda el valor del sector, limpia el sector completo 
y luego reescribe los datos con las modificaciones.

Respecto nuestra aplicacion:
 - Vamos a escribir la hora y el consumo medido cada 2 segundos en una pagina diferente por facilidad de implementacion. 
Por consiguiente, ocuparemos un total de 10 paginas, es decir, algo menos de un sector (maximo 16 paginas)
 - Los datos se guardaran seguidamente: 15:02:45 (8 x char - hora) + ABCD (float - consumo)
 */

//Maximo numero de mensajes que puede contener la cola de comandos de flash
#define QUEUE_MAX						10                //maximum number of objects in a queue

//Definicion de estructura de la memoria
#define nBlock							32	 	//Numero de bloques totales (2MB / 64KB)							//32 blocks in our 16MBytes memory
#define PAGES_FOR_SECTOR				16		//Numero de paginas por sector

//Datos para guardar consumo y hora
#define MAX_CONSUMPTION_DATA			10			//Numero maximo de consumos que guardaremos
#define CONSUMPTION_SIZE_BYTES			4			//Cada consumo ocupa 4 bytes (float)
#define HORA_SIZE_BYTES					4			//Cada hora ocupa 8 char (HH:MM:SS)

//SPI
extern ARM_DRIVER_SPI Driver_SPI4;					//Driver SPI4 externo
static ARM_DRIVER_SPI* SPIdrv = &Driver_SPI4;		//Puntero al driver usado

//Hilo y cola
static osThreadId_t id_thread__flash;           //ID del hilo de Flash
osMessageQueueId_t id_flash_commands_queue;     //Cola de mensajes para comandos de flash
static void thread__flash (void *argument);          //Declaracion del hilo


//Funciones locales
// Inicializaci�n
static void W25Q16_Init_SPI(void);                     // Configuracion de pines y driver SPI
static void SPI_callback(uint32_t event);              // Callback al terminar transmision SPI
static void W25Q16_Init (void);                        // Inicializa la memoria Flash

// Funciones recurrentes
static void W25Q16_WriteInstruction(uint8_t val);      // Escribe una instruccion (1 byte)
static void W25Q16_Reset(void);                        // Reinicia el chip (no borra datos)
static void W25Q16_Erase_64kBlock (uint16_t numBlock); // Borra un bloque de 64 KB
static void W25Q16_Erase_Sector (uint16_t numSector);  // Borra un sector de 4 KB
static uint16_t W25Q16_ReadID(uint8_t number_id);      // Lee el ID del fabricante/dispositivo

static void W25Q16_Read (uint32_t startPage, uint8_t offset, uint32_t size, uint8_t *rData);       // Lectura lenta
static void W25Q16_FastRead (uint32_t startPage, uint8_t offset, uint32_t size, uint8_t *rData);   // Lectura rapida

static void W25Q16_WritePage_Clean (uint32_t page, uint16_t offset, uint32_t size, uint8_t *data); // Escritura tras limpiar pagina

// Control escritura
static void write_enable (void);                       // Habilita escritura
static void write_disable (void);                      // Deshabilita escritura

static uint32_t bytesToWrite (uint32_t size, uint32_t offset); // Caculo de bytes �tiles en pagina
static void erase_usable_memory (void);                         // Borra la memoria que usaremos

//Funciones para bajo consumo (no se emplean)
void W25Q16_PowerDown (void);     // Pone el chip en modo bajo consumo
void W25Q16_PowerUp (void);       // Reactiva el chip desde bajo consumo

//Funciones locales para anadir consumos y horas en flash y recibir todos
static void addConsumption(uint8_t position, float *consumption, char* hora);
//static void getAllConsumptions(float *consumptions, char hour[][FLASH_NUM_CHAR_HORA]); // Lee todos los consumos y los copia
static void getAllConsumptions(float *consumptions, char *hours);

//Funciones para Tests
static void leer_mem_entera(void);              // Lee los primeros 50 bytes para depuraci�n
static void test_write_read(void);              // Prueba de escritura y lectura en bucle

//Init flash
void Init_FlashControl (void) {
  //const static osThreadAttr_t th_attr = {.stack_size = 7000};
	if (id_flash_commands_queue != NULL)
		id_flash_commands_queue = osMessageQueueNew(QUEUE_MAX, sizeof(MSGQUEUE_FLASH_t), NULL);
	if (id_thread__flash != NULL)
		id_thread__flash = osThreadNew(thread__flash, NULL, NULL); 
	if (id_thread__flash == NULL)
	{
      	strncpy(detalleError, "MSG QUEUE ERROR FLASH", sizeof(detalleError) - 1);
        osThreadFlagsSet(id_thread__app_main, FLAG__ERROR);
	}
	if (id_flash_commands_queue == NULL)
	{
       	strncpy(detalleError, "THREAD ERROR FLASH", sizeof(detalleError) - 1);
       	osThreadFlagsSet(id_thread__app_main, FLAG__ERROR);
	}
}

/**
 * @brief Hilo principal de gestion de la memoria Flash. Procesa comandos mediante cola.
 * @param argument No usado.
 */

static void thread__flash (void *argument) {
  MSGQUEUE_FLASH_t flash_msg_rec;
  uint8_t posicion_consumo = 0;

  W25Q16_Init();				// Inicializa SPI y chip
  //erase_usable_memory();	 	// Borra memoria usada al arrancar

	#ifdef TEST_FLASH
  		leer_mem_entera();
  		test_write_read();
	#endif

 	 while (1) {
		if (osMessageQueueGet(id_flash_commands_queue, &flash_msg_rec, NULL, osWaitForever)==osOK) {
			switch (flash_msg_rec.command) 
			{
				case FLASH_CMD__ADD_CONSUMPTION:
					// Guarda un nuevo consumo y avanza de forma circular (maximo 10)
                    addConsumption(posicion_consumo, flash_msg_rec.consumption, flash_msg_rec.hour);
                    posicion_consumo = (posicion_consumo == (MAX_CONSUMPTION_DATA - 1)) ? 0 : (posicion_consumo + 1);
				break;
				case FLASH_CMD__GET_ALL_CONSUMPTION:
					// Copia todos los consumos guardados en memoria
                    getAllConsumptions(flash_msg_rec.consumption, flash_msg_rec.hour);
                    osThreadFlagsSet(id_thread__app_main, FLAG__CONSUMO_READY_FLASH);
				break;
				case FLASH_CMD__ERASE:
                    erase_usable_memory();	//Borra la memoria usada
				break;
			}
		}
	}
}

/**
 * @brief Borra la memoria que sera utilizada para guardar consumos.
 */
static void erase_usable_memory (void) 
{
	for (int i = 0; i < 3; i++) {
		W25Q16_Erase_64kBlock(i);
	}
}

/**
 * @brief Lee los primeros 50 bytes de la memoria para comprobar su contenido.
*/

static void leer_mem_entera(void) 
{
	uint8_t previousData[50];
	W25Q16_FastRead(0, 0, 50, previousData);
	previousData[0] = 0x00;
}

/**
 * @brief Prueba de escritura y lectura continua en la memoria para verificar funcionamiento.
*/

static void test_write_read (void) 
{
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

/**
 * @brief Inicializa los pines y configuraci�n del SPI para la memoria Flash W25Q16.
*/

static void W25Q16_Init_SPI(void)
{
  __HAL_RCC_GPIOE_CLK_ENABLE();
  
  static GPIO_InitTypeDef GPIO_InitStruct_RFID;
  /*CS*/    //SPI_CS -- SPI_B_NSS       PE11 //revisar NAK que pin es? mirar en proyecto
  __HAL_RCC_GPIOE_CLK_ENABLE();
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

/**
 * @brief Callback llamado al finalizar una operaci�n SPI.
 * @param event Evento SPI (TRANSFER_COMPLETE, DATA_LOST, MODE_FAULT)
 */

static void SPI_callback(uint32_t event)
{
  uint32_t error;
    switch (event) {
    case ARM_SPI_EVENT_TRANSFER_COMPLETE:
        error = osThreadFlagsSet(id_thread__flash, TRANSFER_COMPLETE);
        if(error == osFlagsErrorUnknown) {
          __breakpoint(0);
        } else if (error == osFlagsErrorParameter) {
          osThreadFlagsSet(id_thread__flash, TRANSFER_COMPLETE);
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

/**
 * @brief Inicializa el SPI y resetea el chip de memoria Flash (no la borra).
 */

static void W25Q16_Init (void) 
{
	W25Q16_Init_SPI();
	W25Q16_Reset();
}

/**
 * @brief Escribe una instruccion de un solo byte en la memoria (util para comandos simples).
 * @param val Instruccion a enviar.
 */

static void W25Q16_WriteInstruction(uint8_t val) 
{
	//CS low
	HAL_GPIO_WritePin(GPIOE, GPIO_PIN_11, GPIO_PIN_RESET);
	//send address and data
	SPIdrv->Send(&val, 1);
	osThreadFlagsWait(TRANSFER_COMPLETE, osFlagsWaitAny, osWaitForever);
	//CS high
	HAL_GPIO_WritePin(GPIOE, GPIO_PIN_11, GPIO_PIN_SET);
}

/**
 * @brief Realiza un reinicio del chip (sin borrar datos).
 */

static void W25Q16_Reset(void) 
{
	W25Q16_WriteInstruction(ENABLE_RESET);
	W25Q16_WriteInstruction(RESET_DEVICE);
}

/**
 * @brief Lee el ID del fabricante/dispositivo.
 * @param number_id Numero de ID a leer.
 * @return ID del dispositivo.
 */

static uint16_t W25Q16_ReadID(uint8_t number_id) 
{
	uint8_t instuction = READ_ID;
  	uint32_t rx_data;
  	 //CS low
	HAL_GPIO_WritePin(GPIOE, GPIO_PIN_11, GPIO_PIN_RESET);
	 //send address and data
	SPIdrv->Send(&instuction, 1);
	osThreadFlagsWait(TRANSFER_COMPLETE, osFlagsWaitAny, osWaitForever);
	SPIdrv->Receive(&rx_data, 3); //revisar quizas es 4 (no usamos la funcion, asi que nos da igual)
  	osThreadFlagsWait(TRANSFER_COMPLETE, osFlagsWaitAny, osWaitForever);
  	//CS high
	HAL_GPIO_WritePin(GPIOE, GPIO_PIN_11, GPIO_PIN_SET);

  return ((rx_data>>8)&0xFFFF);
}

/**
 * @brief Lee datos de la memoria Flash desde una direccion concreta.
 * @param startPage Pagina inicial (cada pagina = 256 bytes).
 * @param offset Desplazamiento desde el inicio de la pagina.
 * @param size Numero de bytes a leer.
 * @param rData Puntero al buffer donde se guardaran los datos leidos.
 * 
 * Se necesita mandar la instruccion de leer + 24 bit address
 */

static void W25Q16_Read (uint32_t startPage, uint8_t offset, uint32_t size, uint8_t *rData) 
{
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

/**
 * @brief Lee datos rapidamente usando la instruccion FastRead.
 * @param startPage Pagina inicial.
 * @param offset Desplazamiento dentro de la pagina.
 * @param size Numero de bytes a leer.
 * @param rData Puntero al buffer para almacenar datos.
 * 
 * Funciona en la maxima frecuencia posible (50 Hz)
 * A�ade 8 dummy clocks despues de los 24 bits de address (norma del protocolo)
 */

static void W25Q16_FastRead (uint32_t startPage, uint8_t offset, uint32_t size, uint8_t *rData) 
{
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

/**
 * @brief Habilita escritura en la memoria (requerido antes de escribir).
 */

static void write_enable (void) 
{
	W25Q16_WriteInstruction(WRITE_ENABLE);
	osDelay(5);
}

/**
 * @brief Deshabilita la escritura en la memoria. Se necesita usar tras instruccion de escritura y borrado.
 */

static void write_disable (void) 
{
	W25Q16_WriteInstruction(WRITE_DISABLE);
	osDelay(5);
}

/**
 * @brief Borra un bloque de 64 KB.
 * @param numBlock indice del bloque (0 a 31).
 * 
 * El minimo espacio para borrar es un sector (4 KB)
 * Antes de poder borrar memoria es necesario usar la instruccion write_enable
 */

static void W25Q16_Erase_64kBlock (uint16_t numBlock) 
{
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

/**
 * @brief Borra un sector de 4 KB.
 * @param numSector indice del sector dentro del bloque.
 */

static void W25Q16_Erase_Sector (uint16_t numSector) 
{
	uint8_t tData[4];
	uint32_t memAddr = numSector*16*256; //Numero sector * 16 bloques (por sector)* 256 paginas (por bloque)
	
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

/**
 * @brief Calcula cuantos bytes se pueden escribir desde un offset hasta el final de la p�gina.
 * @param size Tamano total de los datos a escribir.
 * @param offset Desplazamiento dentro de la pagina.
 * @return Numero de bytes que se pueden escribir en esa pagina.
 */

static uint32_t bytesToWrite (uint32_t size, uint32_t offset) 
{
	if ((size+offset) < 256) return size;
	else return 256-offset;
}

/**
 * @brief Escribe datos en una pagina despues de limpiar el sector (obligatorio para poder escribir).
 * @param page Pagina de inicio.
 * @param offset Desplazamiento desde el inicio de la pagina.
 * @param size Numero de bytes a escribir.
 * @param data Puntero a los datos a escribir.
 * 
 * Si se excede el numero de bytes de una pagina, se sobreescribiran los datos 
 */

static void W25Q16_WritePage_Clean (uint32_t page, uint16_t offset, uint32_t size, uint8_t *data) 
{
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
	for (uint32_t i = 0; i<numPages; i++) 
	{
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
		for (uint16_t i = 0; i < bytesRemaining; i++) 
		{
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

/**
 * @brief Anade un nuevo valor de consumo en una posicion concreta de memoria.
 * @param position Posicion en memoria circular (0-9).
 * @param consumption Puntero al float de consumo que queremos escribir en flash.
 * @param hour Puntero a la cadena de hora (formato "HH:MM:SS") que queremos escribir en flash.
 
 Llamada a funcion: addConsumption(posicion_consumo, flash_msg_rec.consumption, flash_msg_rec.hour);
 ** flash_msg_rec.consumption = float flash_consumo_tx; con valores de consumo en ese instante
 ** flash_msg_rec.hour = char flash_hora_tx[FLASH_NUM_CHAR_HORA]; con valores de hora en ese instante
 */

static void addConsumption(uint8_t position, float* consumption, char* hour)
{
    uint8_t flash_data[HORA_SIZE_BYTES + CONSUMPTION_SIZE_BYTES];

    // Copiar la hora al inicio del array
    memcpy(flash_data, hour, HORA_SIZE_BYTES);

    // Copiar el float al final del array
    memcpy(&flash_data[HORA_SIZE_BYTES], consumption, CONSUMPTION_SIZE_BYTES);

    // Escribir todo el bloque en la memoria flash
    W25Q16_WritePage_Clean(position * PAGES_FOR_SECTOR, 0, sizeof(flash_data), flash_data);
}


/**
 * @brief Recupera todos los valores de consumo guardados.
 * @param consumptions Puntero al array de floats donde se copiaran los consumos.
 * @param hour Puntero al array donde se copiaran las horas.
 
 Llamada a funcion -> getAllConsumptions(flash_msg_rec.consumption, flash_msg_rec.hour);
 *** flash_msg_data.consumption = medidas_consumo;   ->> float medidas_consumo[NUM_MAX_MUESTRA_CONSUMO];					//puntero a las medidas del consumo que se mostraran en lcd y flash
 *** flash_msg_data.hour = &horas_consumo[0][0];   	 ->> char horas_consumo[FLASH_NUM_CHAR_HORA][NUM_MAX_MUESTRA_CONSUMO];	//puntero a las horas del consumo que se mostraran en lcd y flash
 */

 //revisar NAK eliminar y probar
 
//static void getAllConsumptions(float *consumptions, char hour[][FLASH_NUM_CHAR_HORA])
//{
//    uint8_t flash_data[HORA_SIZE_BYTES + CONSUMPTION_SIZE_BYTES];
//    char horas[MAX_CONSUMPTION_DATA][FLASH_NUM_CHAR_HORA];
//    &horas[0][0] = hour;
//	//Se lee sector por sector (los primeros 10, donde deberian estar todos los consumos y horas) y
//	//se van copiando 1 a 1 al array apuntado por el mensaje.
//    for (uint8_t position = 0; position < MAX_CONSUMPTION_DATA; position++) 
//    {
//        // Leer los 4 bytes de la memoria flash
//        W25Q16_FastRead(position * PAGES_FOR_SECTOR, 0, sizeof(flash_data), flash_data);

//		// Copiar la hora en la fila correspondiente
//        memcpy(horas[position], flash_data, HORA_SIZE_BYTES);

//        // Copiar el float al array de consumos
//        memcpy(&consumptions[position], &flash_data[HORA_SIZE_BYTES], CONSUMPTION_SIZE_BYTES);
    //}
//}


static void getAllConsumptions(float *consumptions, char *hours)
{
    uint8_t flash_data[FLASH_NUM_CHAR_HORA + sizeof(float)];

    for (uint8_t position = 0; position < MAX_CONSUMPTION_DATA; position++) 
    {
        W25Q16_FastRead(position * PAGES_FOR_SECTOR, 0, sizeof(flash_data), flash_data);

        memcpy(&hours[position * FLASH_NUM_CHAR_HORA], flash_data, FLASH_NUM_CHAR_HORA);  //Se copian los datos de hora de 8 en 8 (ya que le pasamos un puntero a )

        memcpy(&consumptions[position], &flash_data[FLASH_NUM_CHAR_HORA], sizeof(float));  //Se copian los datos de consumo en el array que le pasamos
    }
}

/**
 * @brief Pone el chip en modo de bajo consumo.
 */

static void W25Q16_PowerDown (void) 
{
  W25Q16_WriteInstruction(POWER_DOWN);
  osDelay (5);
}

/**
 * @brief Reactiva el chip desde el modo de bajo consumo.
 */

static void W25Q16_PowerUp(void) 
{
  W25Q16_WriteInstruction(POWER_UP);
  osDelay (5);
}


/* POINTER USAGE SUMMARY FOR UNDERSTANDING THE CODE
int *ptr;
int x = 10;

ptr = &x; assigns the memory address of x to the pointer, now the pointer points to where x is located
So if we look at (*ptr), the content of the pointer, it equals 10

*ptr = 20; Assigns the value of 20 to the content of the pointer.
Consequently, now x is worth 20.

*/
