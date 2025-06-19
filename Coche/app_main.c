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

    //Inicializamos leds (verde y rojo - normal)
    leds_activate_mask |= GET_MASK_LED(LED_GREEN);
    leds_activate_mask &= ~GET_MASK_LED(LED_BLUE);
    leds_activate_mask |= GET_MASK_LED(LED_RED);
    
    //Inicializamos controles
    activeCocheControls(true, true, false); //Activamos servos, activamos consumo, desactivamos distancia

    for(;;)
    {
        flags = osThreadFlagsWait(FLAG__MAIN_CONTROL, osFlagsWaitAny, osWaitForever);
        if (flags & FLAG_STATE_NORMAL)
        {
            app_coche_state = STATE__NORMAL;

            //Inicializamos leds (verde y rojo - normal)
            leds_activate_mask |= GET_MASK_LED(LED_GREEN);
            leds_activate_mask &= ~GET_MASK_LED(LED_BLUE);
            leds_activate_mask |= GET_MASK_LED(LED_RED);

            //Activamos/ desactivamos controles
            activeCocheControls(true, true, false); //Activamos servos, activamos consumo, desactivamos distancia

        }
        
        if (flags & FLAG_STATE_BACK_GEAR)
        {
            leds_activate_mask |= GET_MASK_LED(LED_GREEN);
            leds_activate_mask |= GET_MASK_LED(LED_BLUE);
            leds_activate_mask &= ~GET_MASK_LED(LED_RED);
            app_coche_state = STATE__BACK_GEAR;
            
            //Inicializamos control de distancia
            activeCocheControls(true, true, true); //Activamos servos, activamos consumo, activamos distancia
        }
        if (flags & FLAG_STATE__LOW_POWER)
        {
            //Activamos leds (azul - modo Bajo Consumo)            
            leds_activate_mask &= ~GET_MASK_LED(LED_GREEN);
            leds_activate_mask |= GET_MASK_LED(LED_BLUE);
            leds_activate_mask &= ~GET_MASK_LED(LED_RED);

            //Desactivamos todos los controles
            activeCocheControls(false, false, false); //Desactivamos servos, desactivamos consumo, desactivamos distancia
            
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
    init_pulsador();                //Pulsador azul
    Init_LedsControl();             //Control de los leds
    Init_RF_RX();                   //Radiofrecuencia   
    id_thread__app_main = osThreadNew(thread__app_main, NULL, NULL);
    // if (id_thread__app_main == NULL) revisar
    // {
        
    // }
}

void activeCocheControls (bool servomotores, bool consumo, bool distancia)
{
    if (servomotores)
    {
        Init_Servomotors();     //Se inicializan pines (no lleva hilo)
    }
    else
    {
        DeInit_Servomotors();
    }
    if (consumo)
    {
        Init_GetConsumption();      //Se inicializan pines e hilo de control
    }
    else
    {
        DeInit_GetConsumption();
    }
    if (distancia)
    {
        Init_DistanceControl();     //Se inicializa el sensor e hilo de control
    }
    else
    {
        Stop_DistanceControl();
    }
}
