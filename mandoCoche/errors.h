#ifndef __ERRORS_H
#define __ERRORS_H

#include "app_main.h"

#define DRIVER_TIME_WAIT            1000        //Time to wait in driver tasks (threads, queues, timers initializations, etc) (ms)

typedef enum
{
    ERR_CODE__RF_COMMS_LOST         = 0,
    ERR_CODE__THREAD                = 1,
    ERR_CODE__QUEUE                 = 2,
    ERR_CODE__TIMER                 = 3,
    ERR_CODE__MAGNET_NOT_PRESENT    = 4,
    ERR_CODE__INITIALIZATION        = 5,
    MAX_ERROR_NUM
} error_codes_t;

typedef enum
{
    MODULE__RF               = 0,   // 00
    MODULE__RTC                 ,   // 01
    MODULE__LCD                 ,   // 02
    MODULE__ASK_CONSUMPTION     ,   // 03
    MODULE__DIRECTION           ,   // 04
    MODULE__VELOCITY            ,   // 05
    MODULE__ASK_DISTANCE        ,   // 06
    MODULE__JOYSTICK            ,   // 07
    MODULE__ALARM               ,   // 08
    MODULE__WEB                 ,   // 09
    MODULE__APP                 ,   // 10
    MODULE__FLASH               ,   // 11
    MODULE__MAX_MODULES         //12
} modules_type_t;               //max 99 (por display en lcd)


extern char *strErrorModules[MODULE__MAX_MODULES];
extern char *strErrorDescription[MAX_ERROR_NUM];
void push_error (uint8_t error_code, uint8_t module_type, uint8_t error_detail);

#endif
