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
#define DELETE_USER								0x02
#define GET_ALL_USERS							0x04
#define ADD_USER                  0x08
#define ADD_EVENT                 0x10
#define DELETE_EVENTS             0x20
#define GET_ALL_EVENTS            0x40
#define GET_USER									0x80
#define READY											0x01
#define FLAG_POWER_UP             0x100
#define FLAG_POWER_DOWN           0x200



typedef struct {
	uint8_t action;
	uint8_t numUsers;
  uint8_t id[5];
  char name[19];
  uint8_t password[4];
} MSGQUEUE_USER_t;

typedef struct {
	uint8_t action;
	uint8_t numEvents;
  uint8_t year;
  uint8_t month;
  uint8_t day;
  uint8_t hours;
  uint8_t minutes;
  uint8_t seconds;
  char description[19];
} MSGQUEUE_EVENT_t;

typedef struct {
	uint8_t action;
	uint8_t numUsers;
  uint8_t id[5];
  char name[19];
  uint8_t password[4];
	uint8_t numEvents;
  uint8_t year;
  uint8_t month;
  uint8_t day;
  uint8_t hours;
  uint8_t minutes;
  uint8_t seconds;
  char description[19];
} MSGQUEUE_FLASH_t;


osThreadId_t Init_ThFlash (void);

osMessageQueueId_t getInputQueue(void);
osMessageQueueId_t getOutputQueue(void);

void add_user1(void);
void add_user2(void);
void add_user3(void);
void add_user4(void);
void add_event1(void);
#endif /* __MAIN_H */
