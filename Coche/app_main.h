#ifndef __APP_MAIN_H
#define __APP_MAIN_H

#include "cmsis_os2.h"                  // ::CMSIS:RTOS2

//Flags to thread app_main
#define FLAG_STATE_NORMAL       0x0001
#define FLAG_STATE_BACK_GEAR    0x0002
#define FLAG_STATE__LOW_POWER   0x0010

#define FLAG__MAIN_CONTROL (FLAG_STATE_NORMAL       | \
                            FLAG_STATE_BACK_GEAR    )

typedef enum
{
    STATE__NORMAL,
    STATE__BACK_GEAR,
    STATE__LOW_POWER
} app_coche_state_t;

extern osThreadId_t id_thread__app_main;
extern app_coche_state_t app_coche_state;

void Init_AllAppThreads (void);

#endif
