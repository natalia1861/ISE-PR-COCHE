#include "app_main.h"
#include "lcd.h"
#include "RTC.h"
#include "nRF24L01_TX.h"
#include "joystick_control.h"
#include "consumption.h"
#include "nak_led.h"
#include "leds_control.h"
#include "RTC.h"
#include "ask_distance_control.h"
#include "ask_consumption_control.h"

#define NUM_MAX_MUESTRA_CONSUMO     10      //Buffer circular de consumo (memoria flash)
#define MAX_DISTANCE                4096    //Definir el maximo valor para la distancia revisarMSP
#define MIN_DISTANCE                0       //Definir el minimo valor para la distancia revisarMSP

//Variables globales
osThreadId_t id_thread__app_main;

//Variables locales
app_state_t app_state = FIRST_APP_STAGE;

//Funciones locales
lineas_distancia_t calcularLineasDistancia (uint16_t distancia);

//Funcion principal - control del automata y el LCD
void thread__app_main_control (void *no_argument)
{
    uint32_t flags;
    uint8_t medidas_consumo[NUM_MAX_MUESTRA_CONSUMO];
    uint8_t *medidas_consumo_ptr = medidas_consumo;
    uint8_t numero_muestra = 0;
    uint8_t state_enter = true;
    uint8_t state_error = false;
    
    
    while(1)
    {
        osThreadFlagsWait(FLAG__MAIN_CONTROL, osFlagsWaitAny, osWaitForever);
        if (flags & FLAG__PRESS_UP) //Pulsación arriba joystick
        {
            app_state = (app_state == MAX_APP_STATE) ? FIRST_APP_STAGE : (app_state_t) (app_state + 1);
            state_enter = true;
        }
        
        if (flags & FLAG__ERROR) //Flag de error
        {
            state_error = true;
            LCD_write(LCD_LINE__ONE, "ERROR...");
            LCD_write(LCD_LINE__TWO, "DETAIL"); //revisarNAK añadir detalle de error
        }
        
        if (flags & FLAG__PRESS_CENTER) //Se sale del estado de error tras presionar el boton central - se vuelve al inicio
        {
            if (state_error)
            {
                state_error = false;
                app_state = FIRST_APP_STAGE;
                state_enter = true;
            }
        }
        
        if (flags & FLAG__ENTER_LOW_POWER)
        {
            app_state = (app_state == APP_STAGE__LOW_POWER) ? APP_STATE__NORMAL : APP_STAGE__LOW_POWER;
        }
         
        if (!state_error)
        {
            switch (app_state)
            {
                case APP_STATE__NORMAL:               
                    if (state_enter)
                    {
                        //Desactivamos todos los controles de otros estados
                        Stop_askDistanceControl();
                        Stop_askConsumptionControl();
                        
                        //Mostramos estado inicial de LEDs
                        leds_activate_mask |= GET_MASK_LED(LED_GREEN);
                        leds_activate_mask &= ~GET_MASK_LED(LED_BLUE);
                        leds_activate_mask &= ~GET_MASK_LED(LED_RED);
                        
                        //Mostramos por el LCD que estamos en modo normal de funcionamiento
                        LCD_write (LCD_LINE__ONE, "State: Normal");
                        state_enter = false;
                    }
                
                    if (flags & FLAG__MOSTRAR_HORA)
                    {
                        //Mostramos la hora en el LCD
                        LCD_write (LCD_LINE__TWO, rtc_date_time[RTC_HOUR]);
                    }
                    //En caso de error, se muestra en LCD
                    break;
                    
                case APP_STAGE__LOW_POWER:               
                    if (state_enter)
                    {
                        //Desactivamos todos los controles de otros estados
                        Stop_askDistanceControl();
                        Stop_askConsumptionControl();
                        
                        //Mostramos estado inicial de LEDs
                        leds_activate_mask &= ~GET_MASK_LED(LED_GREEN);
                        leds_activate_mask |= GET_MASK_LED(LED_BLUE);
                        leds_activate_mask &= ~GET_MASK_LED(LED_RED);
                        
                        //Mostramos por el LCD que estamos en modo de bajo consumo
                        LCD_write (LCD_LINE__ONE, "State: Low Power");
                        state_enter = false;
                    }
                
                    if (flags & FLAG__MOSTRAR_HORA)
                    {
                        //Mostramos la hora en el LCD - linea 2
                        LCD_write (LCD_LINE__TWO, rtc_date_time[RTC_HOUR]);
                    }
                    break;
                    
                case APP_STAGE__BACK_GEAR:
                    if (state_enter)
                    {
                        //Desactivamos todos los controles de otros estados
                        Stop_askDistanceControl();
                        Stop_askConsumptionControl();
                        
                        //Mostramos estado inicial de LEDs
                        leds_activate_mask &= ~GET_MASK_LED(LED_GREEN);
                        leds_activate_mask &= ~GET_MASK_LED(LED_BLUE);
                        leds_activate_mask |= GET_MASK_LED(LED_RED);
                        
                        //Mostramos por el LCD que estamos en modo de bajo consumo
                        LCD_write (1, "State: Back Gear");
                        state_enter = false;
                        
                        //Creamos el hilo de solicitud de distancia cada x tiempo que mandará comando de solicitud de distancia revisarNAK revisarMSP
                        Init_askDistanceControl();
                    }
                    
                    osDelay (2000); //Espera 2 segundos para visualizar que se entró al estado de marcha atrás
                    if (flags & FLAG__MOSTRAR_DISTANCIA) //Flag enviado desde nRF TX tras recibir la distancia
                    {
                        //Se muestra la distancia por el LCD
                        LCD_mostrarLineasDistancia(calcularLineasDistancia(nRF_data_received.distancia));
                    }
                    break;
                    
                case APP_STAGE__MOSTRAR_CONSUMO:
                    if (state_enter)
                    {
                        //Desactivamos todos los controles de otros estados
                        Stop_askDistanceControl();
                        
                        //Mostramos estado inicial de LEDs
                        leds_activate_mask |= GET_MASK_LED(LED_GREEN);
                        leds_activate_mask |= GET_MASK_LED(LED_BLUE);
                        leds_activate_mask |= GET_MASK_LED(LED_RED);
                        
                        //Mostramos en la línea 1 del LCD que estamos en el modo de mostrar consumo
                        LCD_write (LCD_LINE__ONE, "State: Consumpion");
                    }
                    
                    if (flags & FLAG__PRESS_RIGHT) //Mostramos el consumo de la flash en el LCD
                    {
                        //actualizamos el numero de la muestra que mostramos
                        numero_muestra = (numero_muestra == NUM_MAX_MUESTRA_CONSUMO - 1) ? 0 : numero_muestra + 1; //revisar variables de puntero
                        medidas_consumo_ptr = medidas_consumo + numero_muestra;
                        
                        //Mostramos por el LCD la muestra indicada
                        LCD_mostrarConsumo(numero_muestra, *medidas_consumo_ptr);
                    }
                    break;
            }
        }
    }
}

void Init_AllAppThreads(void)
{
    /* Init all threads here*/
    id_thread__app_main = osThreadNew(thread__app_main_control, NULL, NULL);
    //Init_RTC_Update();
    //Init_JoystickControl();
    //Init_thread_GetConsumption();
    //Init_LedsControl();
    Init_RF_TX();
    //osThreadNew(app_main, NULL, &app_main_attr);
    netInitialize();
    Init_askDistanceControl();
}

lineas_distancia_t calcularLineasDistancia (uint16_t distancia)
{
    uint32_t lines;
    if (distancia <= MIN_DISTANCE)
    {
        distancia = MIN_DISTANCE;
    }
    else if (distancia >= MAX_DISTANCE)
    {
        distancia = MAX_DISTANCE;
    }

    lines = ((distancia* (LCD_MAX_LINES - LCD_MIN_LINES))/(MAX_DISTANCE - MIN_DISTANCE));

    return (lineas_distancia_t) lines;
}
