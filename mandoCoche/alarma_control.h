#include "cmsis_os2.h"

#ifndef __ALARMA_CONTROL_H
#define __ALARMA_CONTROL_H


#define FLAG_DEACTIVATE_ALARM       0x001
#define FLAG_DIST_ALTA              0x002
#define FLAG_DIST_MEDIA             0x004
#define FLAG_DIST_BAJA              0x008
#define FLAG_TONO                   0x010

#define FLAGS_ALARMA               (FLAG_DIST_ALTA              | \
                                    FLAG_DIST_MEDIA             | \
                                    FLAG_DIST_BAJA              | \
                                    FLAG_DEACTIVATE_ALARM       | \
                                    FLAG_TONO                   )

#define SET_ALARM_FLAG(lineas)      (1 << lineas)       //Macro que permite obtener la flag necesaria a activar segun las lineas calculadas de distancia a mostrar en LCD

//Variables globales
extern osThreadId_t id_thread__AlarmaControl;

//INICIAR THREAD PWM Y TEST
void Init_AlarmaControl (void);
void Deinit_AlarmaControl (void);

#endif
