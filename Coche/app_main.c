#include "app_main.h"
#include "nRF24L01_RX.h"
#include "leds_control.h"
#include "nak_led.h"

void thread__app_main (void *no_argument)
{
    //Activamos led verde - application led
    leds_activate_mask |= GET_MASK_LED(LED_GREEN);
    for(;;)
    {
        
    }
}

void Init_AllAppThreads (void)
{
    Init_LedsControl();
    Init_RF_RX();
    osThreadNew(thread__app_main, NULL, NULL);
}
