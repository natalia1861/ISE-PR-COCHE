#include "app_main.h"
#include <string.h>
#include "lcd.h"
#include "RTC.h"
#include "nRF24L01_TX.h"
#include "joystick_control.h"
#include "nak_led.h"
#include "leds_control.h"
#include "RTC.h"
#include "ask_distance_control.h"
#include "ask_consumption_control.h"
#include "velocity_control.h"
#include "tm_stm32_nrf24l01.h"
#include "sensor_AS5600.h"
#include "direction_control.h"

#define NUM_MAX_MUESTRA_CONSUMO     10      //Buffer circular de consumo (memoria flash)
#define MAX_DISTANCE                4096    //Definir el maximo valor para la distancia revisarMSP
#define MIN_DISTANCE                0       //Definir el minimo valor para la distancia revisarMSP

//Variables globales
osThreadId_t id_thread__app_main;
char detalleError[20] = {0};
app_state_t app_state = FIRST_APP_STAGE;

//Funciones locales
lineas_distancia_t calcularLineasDistancia (uint16_t distancia);
nRF_data_transmitted_t nRF_data = {0};

//Funciones locales
void Send_CMD_StateChange (app_state_t app_state);

//Funcion principal - control del automata y el LCD
void thread__app_main_control (void *no_argument)
{
    uint32_t flags;
    uint8_t medidas_consumo[NUM_MAX_MUESTRA_CONSUMO];
    uint8_t *medidas_consumo_ptr = medidas_consumo;
    uint8_t numero_muestra = 0;
    uint8_t state_enter = true;
    uint8_t state_error = false;
    
    //Inicializamos el LCD
    LCD_start();
    
    //ESTADO INICIAL
    //Inicializamos controles iniciales
    Init_askConsumptionControl();
    
    //Mostramos estado inicial de LEDs
    leds_activate_mask |= GET_MASK_LED(LED_GREEN);
    leds_activate_mask &= ~GET_MASK_LED(LED_BLUE);
    leds_activate_mask &= ~GET_MASK_LED(LED_RED);
    
    //Mostramos por el LCD que estamos en modo normal de funcionamiento
    LCD_write (LCD_LINE__ONE, "State: Normal");
    state_enter = false;
    
    while(1)
    {
        flags = osThreadFlagsWait(FLAG__MAIN_CONTROL, osFlagsWaitAny, osWaitForever);
        
/************Flags comunes a todos los estados*************************************************/
        
        if (flags & FLAG__PRESS_UP) //Pulsación arriba joystick
        {
            LCD_clean();
            app_state = (app_state == MAX_APP_STATE) ? FIRST_APP_STAGE : (app_state_t) (app_state + 1);
            state_enter = true;
        }
        
        if (flags & FLAG__CONSUMO_EN_FLASH)
        {
            //revisarNAK guardar consumo en memoria flash
            //nRF_data_received.consumo;
        }
        
        if (flags & FLAG__ERROR) //Flag de error
        {
            //Paramos todos los controles
            Stop_askConsumptionControl();
            Stop_askDistanceControl();
            
            leds_activate_mask |= GET_MASK_LED(LED_GREEN);
            leds_activate_mask |= GET_MASK_LED(LED_BLUE);
            leds_activate_mask |= GET_MASK_LED(LED_RED);

            state_error = true;
            LCD_write(LCD_LINE__ONE, "ERROR...");
            LCD_write(LCD_LINE__TWO, detalleError); //revisarNAK añadir detalle de error
        }
        
        if (flags & FLAG__PRESS_CENTER) //Se sale del estado de error tras presionar el boton central - se vuelve al inicio
        {
            if (state_error)
            {
                state_error = false;
                app_state = FIRST_APP_STAGE;
                
                //ESTADO INICIAL
                //Inicializamos controles iniciales
                Init_askConsumptionControl();
                
                //Mostramos estado inicial de LEDs
                leds_activate_mask |= GET_MASK_LED(LED_GREEN);
                leds_activate_mask &= ~GET_MASK_LED(LED_BLUE);
                leds_activate_mask &= ~GET_MASK_LED(LED_RED);
                
                //Mostramos por el LCD que estamos en modo normal de funcionamiento
                LCD_write (LCD_LINE__ONE, "State: Normal");
                state_enter = false;
            }
        }
        
        if (flags & FLAG__ENTER_LOW_POWER)
        {
            app_state = (app_state == APP_STAGE__LOW_POWER) ? APP_STATE__NORMAL : APP_STAGE__LOW_POWER;
        }
         
/*****************************************************************************************************************************/
        
/************Comienzo de control de los estados*******************************************************************************/
        if (!state_error)
        {
            switch (app_state)
            {
                case APP_STATE__NORMAL:               
                    if (state_enter)
                    {
                        //Desactivamos todos los controles de otros estados
                        Stop_askDistanceControl();
                        
                        //Enviamos el estado al coche para habilitar/deshabilitar controles
                        Send_CMD_StateChange(app_state);
                        
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
                        
                        //Enviamos el estado al coche para habilitar/deshabilitar controles
                        Send_CMD_StateChange(app_state);
                        
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
                        //Enviamos el estado al coche para habilitar/deshabilitar controles
                        Send_CMD_StateChange(app_state);
                        
                        //Mostramos estado inicial de LEDs
                        leds_activate_mask &= ~GET_MASK_LED(LED_GREEN);
                        leds_activate_mask &= ~GET_MASK_LED(LED_BLUE);
                        leds_activate_mask |= GET_MASK_LED(LED_RED);
                        
                        //Mostramos por el LCD que estamos en modo de bajo consumo
                        LCD_write (1, "State: Back Gear");
                        
                        //Creamos el hilo de solicitud de distancia cada x tiempo que mandará comando de solicitud de distancia revisarNAK revisarMSP
                        Init_askDistanceControl();
                        
                        state_enter = false;
                    }
                    
                    osDelay (1000); //Espera 2 segundos para visualizar que se entró al estado de marcha atrás
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
                        
                        //Enviamos el estado al coche para habilitar/deshabilitar controles
                        Send_CMD_StateChange(app_state);
                        
                        //Mostramos estado inicial de LEDs
                        leds_activate_mask |= GET_MASK_LED(LED_GREEN);
                        leds_activate_mask &= ~GET_MASK_LED(LED_BLUE);
                        leds_activate_mask &= ~GET_MASK_LED(LED_RED);
                        
                        //Mostramos en la línea 1 del LCD que estamos en el modo de mostrar consumo
                        LCD_write (LCD_LINE__ONE, "State: Consumpion");
                        
                        //Reiniciamos la muestra
                        numero_muestra = 0;
                        medidas_consumo_ptr = medidas_consumo;
                        
                        //Mostramos por el LCD la muestra indicada
                        LCD_mostrarConsumo(numero_muestra, *medidas_consumo_ptr);
                        
                        state_enter = false;
                    }
                    
                    if (flags & FLAG__PRESS_RIGHT) //Mostramos el consumo de la flash en el LCD
                    {
                        //actualizamos el numero de la muestra que mostramos
                        numero_muestra = (numero_muestra == NUM_MAX_MUESTRA_CONSUMO - 1) ? 0 : numero_muestra + 1;
                        medidas_consumo_ptr = medidas_consumo + numero_muestra;
                        
                        //Mostramos por el LCD la muestra indicada
                        LCD_mostrarConsumo(numero_muestra, *medidas_consumo_ptr);
                    }
                    
                    if (flags & FLAG__PRESS_LEFT) //Mostramos el consumo de la flash en el LCD
                    {
                        //actualizamos el numero de la muestra que mostramos
                        numero_muestra = (numero_muestra == 0) ? (NUM_MAX_MUESTRA_CONSUMO - 1) : numero_muestra - 1;
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
    Init_RF_TX();
    Init_LedsControl();
    Init_RTC_Update();
    Init_JoystickControl();
    Init_VelocityCointrol();
    //Init_AS5600_Thread();
    Init_DirectionControl();
    //osThreadNew(app_main, NULL, &app_main_attr); WEB
    //netInitialize(); WEB
}

lineas_distancia_t calcularLineasDistancia(uint16_t distancia)
{
    if (distancia <= MIN_DISTANCE)
        return LCD_LINE__NO_LINE;

    if (distancia >= MAX_DISTANCE)
        return LCD_MAX_LINES;

    // Dividimos el rango [1, 4095] en 4 tramos iguales
    uint32_t tramo = (MAX_DISTANCE - 1) / (LCD_MAX_LINES - LCD_MIN_LINES);

    //Se calcula el bloque al que pertenece (numero de lineas)
    uint32_t lines = (distancia - 1) / tramo + 1;

    return (lineas_distancia_t)lines;
}

//Utilizado para habilitar y deshabilitar controles en el coche (distancia)
void Send_CMD_StateChange (app_state_t app_state)
{
    if (app_state == APP_STAGE__BACK_GEAR)
    {
        nRF_data.command = nRF_CMD__BACK_GEAR_MODE;
    }
    else
    {
        nRF_data.command = nRF_CMD__NORMAL_MODE;
    }
    if (osMessageQueuePut(id_queue__nRF_TX_Data, &nRF_data, NULL, 1000) != osOK)
    {
        strncpy(detalleError, "MSG QUEUE ERROR        ", sizeof(detalleError) - 1);
        osThreadFlagsSet(id_thread__app_main, FLAG__ERROR);
    }
}
