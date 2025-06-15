#include "errors.h"
#include <stdio.h>
#include <string.h>

//Texto que se muestra por pantalla asociado a los codigos de errores definidos
char *strErrorDescription[MAX_ERROR_NUM] =
{  //0123456789012345
    "ERR COMMS RF    ",  //ERR_CODE__RF_COMMS_LOST
    "ERR THREAD      ",  //ERR_CODE__THREAD_NOT_CREATED
    "ERR QUEUE       ",  //ERR_CODE__QUEUE_NOT_CREATED
    "ERR TIMER       ",  //ERR_CODE__TIMER_NOT_CREATED
    "ERR MAGNET DET  ",  //ERR_CODE__MAGNET_NOT_PRESENT
    "ERR INIT        ",  //ERR_CODE__INITIALIZATION
    "ERR DATA CORRUPT",  //ERR_CODE__DATA_CORRUPT
};

char *strErrorModules[MODULE__MAX_MODULES] =
{
   //0123456789012345
    "Mod: RF         ",  // MODULE__RF
    "Mod: RTC        ",  // MODULE__RTC
    "Mod: LCD        ",  // MODULE__LCD
    "Mod: Consmp     ",  // MODULE__ASK_CONSUMPTION
    "Mod: Dir        ",  // MODULE__DIRECTION
    "Mod: Vel        ",  // MODULE__VELOCITY
    "Mod: Dist       ",  // MODULE__ASK_DISTANCE
    "Mod: Joy        ",  // MODULE__JOYSTICK
    "Mod: Alarm      ",  // MODULE__ALARM
    "Mod: Web        ",  // MODULE__WEB
    "Mod: APP        ",  // MODULE__APP
    "Mod: Flash      ",  // MODULE__FLASH
};

void push_error(uint8_t module_type, uint8_t error_code, uint8_t error_detail)
{
    // Copiamos el modulo directamente (ya lleva el texto "Mod: ")
    if (module_type < MODULE__MAX_MODULES)
    {
        strncpy(moduloError, strErrorModules[module_type], LCD_MAX_CHARACTERS - 1);
        moduloError[LCD_MAX_CHARACTERS - 1] = '\0'; // Seguridad
    }

    // Creamos el texto de error: "ERR xxx <num>"
    if (error_code < MAX_ERROR_NUM)
    {
        // Copiamos el texto base del error
        strncpy(detalleError, strErrorDescription[error_code], LCD_MAX_CHARACTERS - 2); // deja hueco para el número
        detalleError[LCD_MAX_CHARACTERS - 2] = '\0'; // Por seguridad

        // Añadimos el numero de detalle al final (max 1 dígito) - para distinguir los diferentes errores y trackear mejor 
        char numStr[2] = {0};
        snprintf(numStr, sizeof(numStr), "%u", error_detail);

        strncat(detalleError, numStr, LCD_MAX_CHARACTERS - strlen(detalleError) - 1);
    }

    //Mandamos flag a thread__app_main_control
    switch (error_code)
    {
    case ERR_CODE__RF_COMMS_LOST:   //Flag de perdida de comunicaciones. Se escribe mensaje por LCD, si vuelve, desaparece el mensaje.
        osThreadFlagsSet(id_thread__app_main, FLAG__RF_LOST_COMMS_ERROR);
        break;
    
    case ERR_CODE__THREAD:  //Flag de error de driver -> Error catastrófico. No se permite aceptar, solo reiniciar el sistema.
    case ERR_CODE__QUEUE:
    case ERR_CODE__TIMER:
    case ERR_CODE__INITIALIZATION:
        osThreadFlagsSet(id_thread__app_main, FLAG__DRIVER_ERROR);
        break;

    case ERR_CODE__MAGNET_NOT_PRESENT: //revisar falta gestionar cuando se haga lo de detectar el iman
        osThreadFlagsSet(id_thread__app_main, FLAG__DIR_MAG_NOT_PRES);
        break;
    default:
        break;
    }
}
