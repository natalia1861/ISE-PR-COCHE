#include "cmsis_os2.h"

#ifndef __PWM_H
#define __PWM_H

#define MSGQUEUE_OBJECTS 16 
#define FREQ_TONO1 4000
#define FREQ_TONO2 6000

//INICIAR THREAD PWM Y TEST
int Init_PWM (void);
extern osThreadId_t tid_PWM;
void PWM_init(void);
void PWM_cambio(uint32_t frecuencia);
void PWM_desactivar();
void PWM_activar();
void dist_Cerca();
void dist_Lejos();
void dist_Media();
void tono_dist_pequena();
void tono_dist_med ();
void tono_dist_gran ();
#endif
