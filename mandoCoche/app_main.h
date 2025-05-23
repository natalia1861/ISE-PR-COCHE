#ifndef __APP_MAIN_H
#define __APP_MAIN_H

#include "cmsis_os2.h"

//Flags to thread__joystick_control
#define FLAG__PRESS_UP              0x0001
#define FLAG__PRESS_DOWN            0x0002
#define FLAG__PRESS_RIGHT           0x0004
#define FLAG__PRESS_LEFT            0x0008
#define FLAG__PRESS_CENTER          0x0010
#define FLAG__CARGAR_CONSUMOS       0x0020
#define FLAG__MOSTRAR_HORA          0x0040
#define FLAG__MOSTRAR_DISTANCIA     0x0080
#define FLAG__ENTER_LOW_POWER       0x0100
#define FLAG__CONSUMO_EN_FLASH      0x0200
#define FLAG__CONSUMO_READY_FLASH   0x0400
#define FLAG__ERROR                 0x0800

#define FLAG__MAIN_CONTROL  (FLAG__PRESS_UP             | \
                             FLAG__PRESS_DOWN           | \
                             FLAG__PRESS_RIGHT          | \
                             FLAG__PRESS_LEFT           | \
                             FLAG__PRESS_CENTER         | \
                             FLAG__CARGAR_CONSUMOS      | \
                             FLAG__MOSTRAR_HORA         | \
                             FLAG__MOSTRAR_DISTANCIA    | \
                             FLAG__ENTER_LOW_POWER      | \
                             FLAG__CONSUMO_EN_FLASH     | \
                             FLAG__CONSUMO_READY_FLASH  | \
                             FLAG__ERROR                )

typedef enum
{
    APP_STATE__NORMAL = 0,          //FIRST_APP_STAGE
    APP_STAGE__BACK_GEAR,    
    APP_STAGE__MOSTRAR_CONSUMO,     //MAX_APP_STATE
    APP_STAGE__LOW_POWER            //Se deja fuera porque se entrara desde el boton azul (no joystick como el resto)
} app_state_t;  

#define FIRST_APP_STAGE     APP_STATE__NORMAL
#define MAX_APP_STATE       APP_STAGE__MOSTRAR_CONSUMO

extern osThreadId_t id_thread__app_main;
extern char detalleError[20];
extern app_state_t app_state;

void Init_AllAppThreads(void);

#endif
