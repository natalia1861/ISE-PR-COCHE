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

//Funciones internas
void activeCocheControls (bool servomotores, bool consumo, bool distancia, bool radiofrecuencia);

void thread__app_main (void *no_argument)
{
    uint32_t flags;
    
    app_coche_state = STATE__NORMAL;

    //Inicializamos leds (verde y rojo - normal)
    leds_activate_mask |= GET_MASK_LED(LED_GREEN);
    leds_activate_mask &= ~GET_MASK_LED(LED_BLUE);
    leds_activate_mask |= GET_MASK_LED(LED_RED);

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
            activeCocheControls(true, true, false, true); //Activamos servos, activamos consumo, desactivamos distancia, activamos radiofrecuencia

        }
        
        if (flags & FLAG_STATE_BACK_GEAR)
        {
            leds_activate_mask |= GET_MASK_LED(LED_GREEN);
            leds_activate_mask |= GET_MASK_LED(LED_BLUE);
            leds_activate_mask &= ~GET_MASK_LED(LED_RED);
            app_coche_state = STATE__BACK_GEAR;
            
            //Inicializamos control de distancia
            activeCocheControls(true, true, true, true); //Activamos servos, activamos consumo, activamos distancia, activamos radiofrecuencia
        }
        if (flags & FLAG_STATE__LOW_POWER)
        {
            //Activamos leds (azul - modo Bajo Consumo)            
            leds_activate_mask &= ~GET_MASK_LED(LED_GREEN);
            leds_activate_mask |= GET_MASK_LED(LED_BLUE);
            leds_activate_mask &= ~GET_MASK_LED(LED_RED);

            //Desactivamos todos los controles
            activeCocheControls(false, false, false, false); //Desactivamos servos, desactivamos consumo, desactivamos distancia, , desactivamos radiofrecuencia
            
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
    Init_LedsControl();             //Control de los leds -> siempre activos  
  
    //Inicializamos distancia porque tarda 6 segundos aprox en funcionar
    Init_SensorDistancia();  

    //Inicializamos el hilo principal
    id_thread__app_main = osThreadNew(thread__app_main, NULL, NULL);
    
    //Inicializamos controles / hilos
    activeCocheControls(true, true, false, true); //Activamos servos, activamos consumo, desactivamos distancia, activamos Radiofrecuencia
}

void activeCocheControls (bool servomotores, bool consumo, bool distancia, bool radiofrecuencia)
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
    if (radiofrecuencia)
    {
        Init_RF_RX();               //Se inicializan pines, sensor e hilo de control
    }
    else
    {
        DeInit_RF_RX();
    }
}
