#ifndef __THFLASH_H
#define __THFLASH_H

/* Includes ------------------------------------------------------------------*/
#include "Driver_SPI.h"
#include "stm32f4xx_hal.h"
#include "cmsis_os2.h"                          // CMSIS RTOS header file
#include "string.h"

//INSTRUCTIONS
#define ENABLE_RESET									0x66
#define RESET_DEVICE									0x99
#define READ_ID												0x9F
#define READ_DATA											0x03
#define READ_FAST											0x0B
#define WRITE_ENABLE									0x06
#define WRITE_DISABLE									0x04
#define ERASE_BLOCK									  0xD8
#define ERASE_SECTOR                  0X20
#define PAGE_PROGRAM									0x02
#define POWER_DOWN                    0xB9
#define POWER_UP                      0xAB

//FLAGS
#define TRANSFER_COMPLETE 				0x01              //internal flag used by SPI to know that a single transfer has been completed

typedef enum
{
  FLASH_CMD__ADD_CONSUMPTION,
  FLASH_CMD__GET_ALL_CONSUMPTION,
  FLASH_CMD__ERASE
} cmd_flash_t;

typedef struct 
{
	cmd_flash_t command;
    float* consumption;
} MSGQUEUE_FLASH_t;

void Init_FlashControl (void);
extern osMessageQueueId_t id_flash_commands_queue;
 

#endif /* __MAIN_H */
