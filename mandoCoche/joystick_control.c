#include "joystick_control.h"
#include "gpio.h"
#include "app_main.h"
#include "errors.h"

//Thread
osThreadId_t id_thread__joystick_control = NULL;
void thread__Joystick_control (void *no_argument);

//timer rebotes
static osTimerId_t id_timer_rebotes;
static void timer_rebotes_callback (void *arg);

//timer pulsacion larga
static osTimerId_t id_timer_pulsacion_larga;
static void timer_pulsacion_larga_callback (void *arg);

void thread__Joystick_control (void *no_argument)
{
    uint32_t flags;
    init_Joystick();
    
    id_timer_rebotes = osTimerNew(timer_rebotes_callback, osTimerOnce, NULL, NULL);
    id_timer_pulsacion_larga = osTimerNew(timer_pulsacion_larga_callback, osTimerOnce, NULL, NULL);

    for (;;)
    {
        flags = osThreadFlagsWait(FLAG_PULSACION | FLAG_FIN_REB | FLAG_PL, osFlagsWaitAny, osWaitForever);
		if (flags & FLAG_PULSACION) //Se ha detectado una interrupcion de algun pin de joystick
        {
            osTimerStart(id_timer_rebotes, 50);
            osTimerStart(id_timer_pulsacion_larga, 1000);
        }
		if (flags & FLAG_FIN_REB)   //Se ha terminado el tiempo para considerar rebotes
        {
			if (HAL_GPIO_ReadPin(joy_pin_pulsado.port, joy_pin_pulsado.pin) == GPIO_PIN_RESET)  //Se mira el estado del pin
             {
				switch (joy_pin_pulsado.pin) 
                {
					case GPIO_PIN_10: //UP
                        osThreadFlagsSet(id_thread__app_main, FLAG__PRESS_UP);
					break;                                                  
					case GPIO_PIN_11: //RIGHT                               
                        osThreadFlagsSet(id_thread__app_main, FLAG__PRESS_RIGHT);
					break;                                                  
					case GPIO_PIN_12: //DOWN                                
                        osThreadFlagsSet(id_thread__app_main, FLAG__PRESS_DOWN);
					break;                                                  
					case GPIO_PIN_14: //LEFT                                
                        osThreadFlagsSet(id_thread__app_main, FLAG__PRESS_LEFT);
					break;                                                  
					case GPIO_PIN_15: //CENTER                              
                        osThreadFlagsSet(id_thread__app_main, FLAG__PRESS_CENTER);
					break;
				}
                osTimerStop(id_timer_pulsacion_larga);
                //msg_joy.largo_corto = 0;
			}
            else 
            {
                osTimerStart(id_timer_rebotes, 50);
			}
        }
		if (flags & FLAG_PL) //La pulsacion larga no se gestiona -> siempre es corta
        {
            //me da igual si es largo o corto realmente
            //msg_joy.largo_corto = 1;
		}
    }
}

void timer_rebotes_callback (void *arg) {
	osThreadFlagsSet(id_thread__joystick_control, FLAG_FIN_REB);
}

void timer_pulsacion_larga_callback (void *arg) {
	osThreadFlagsSet(id_thread__joystick_control, FLAG_PL);
}

void Init_JoystickControl (void)
{
    if (id_thread__joystick_control == NULL)
        id_thread__joystick_control = osThreadNew(thread__Joystick_control, NULL, NULL);

    if (id_thread__joystick_control == NULL)
        push_error(MODULE__JOYSTICK, ERR_CODE__THREAD, 0);
}

