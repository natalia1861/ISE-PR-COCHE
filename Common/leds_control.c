#include "leds_control.h"
#include "nak_led.h"
#include "cmsis_os2.h"

uint8_t leds_activate_mask = 0;

void thread__ledsControl (void *no_argument)
{
    INITIALIZE_LEDS();
    for(;;)
    {
        //Se emplea una mascara global para ir actualizando el estado de los LEDs
        if (leds_activate_mask & (GET_MASK_LED(LED_GREEN)))
        {
            LED_GREEN_ON();
        }
        if (leds_activate_mask & (GET_MASK_LED(LED_BLUE)))
        {
            LED_BLUE_ON();
        }
        if (leds_activate_mask & (GET_MASK_LED(LED_RED)))
        {
            LED_RED_ON();
        }
        osDelay(200);
        
        LED_GREEN_OFF();
        LED_BLUE_OFF();
        LED_RED_OFF();
        
        osDelay(200);
    }
}

void Init_LedsControl(void)
{
    osThreadNew(thread__ledsControl, NULL, NULL);
}
