#include "app_main.h"
#include "nRF24L01_RX.h"
#include "leds_control.h"
#include "nak_led.h"
#include "distance_control.h"
#include <stdio.h>
#include "servomotor.h"
#include "consumo_control.h"
#include "gpio.h"
#include "modo_sleep.h"

//Variables globales
app_coche_state_t app_coche_state;
osThreadId_t id_thread__app_main;

void thread__app_main (void *no_argument)
{
    uint32_t flags;
    
    app_coche_state = STATE__NORMAL;
    
    //Activamos led verde - application led
    leds_activate_mask |= GET_MASK_LED(LED_GREEN);
    
    //Inicializamos controles
    Init_Servomotors();
    Init_GetConsumption();

    for(;;)
    {
        flags = osThreadFlagsWait(FLAG__MAIN_CONTROL, osFlagsWaitAny, osWaitForever);
        if (flags & FLAG_STATE_NORMAL)
        {
            app_coche_state = STATE__NORMAL;
            
            //Paramos el control de distancia
            Stop_DistanceControl();
            
            //Despertamos el coche por si acaso venimos de modo bajo consumo
            if(__HAL_PWR_GET_FLAG(PWR_FLAG_SB) != RESET)
            {
            __HAL_PWR_CLEAR_FLAG(PWR_FLAG_SB);
            }
            ETH_PhyExitFromPowerDownMode();
        }
        
        if (flags & FLAG_STATE_BACK_GEAR)
        {
            app_coche_state = STATE__BACK_GEAR;
            
            //Inicializamos control de distancia
            Init_DistanceControl();
        }
        if (flags & FLAG_STATE__LOW_POWER)
        {
            //Entramos en modo bajo consumo
            SleepMode_Measure();
            ETH_PhyEnterPowerDownMode();
        }
    }
}

void Init_AllAppThreads (void)
{
    //Inicializamos modo bajo consumo
    __HAL_RCC_PWR_CLK_ENABLE();
    
    //Inicializamos el resto de contoles
    init_pulsador();
    Init_LedsControl();
    Init_SensorDistancia();
    Init_RF_RX();
    id_thread__app_main = osThreadNew(thread__app_main, NULL, NULL);
    // if (id_thread__app_main == NULL) revisar
    // {
        
    // }
}
