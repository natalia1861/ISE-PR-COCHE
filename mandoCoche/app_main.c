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
#include "flash.h"

#define NUM_MAX_MUESTRA_CONSUMO     10      //Buffer circular de consumo (memoria flash)
#define MAX_RANGE_DISTANCE          500     //Definir el valor en sensor que significa minima distancia
#define MIN_RANGE_DISTANCE          0       //Definir el valor en sensor que significa maxima distancia
#define DISTANCE_SENSIBILITY        25

//Variables globales
osThreadId_t id_thread__app_main;
char detalleError[20] = {0};
app_state_t app_state = FIRST_APP_STAGE;

//Message to nRF queue
nRF_data_transmitted_t nRF_data = {0};

//Message to flash queue (tanto para enviar como para recibir datos)
MSGQUEUE_FLASH_t flash_msg_data;

//Comunicacion con Web
char consumo_S [80];
char distancia_S[80];

//Funciones locales
lineas_distancia_t calcularLineasDistancia (uint16_t distancia);

//Funciones locales
void Send_CMD_StateChange (app_state_t app_state);
void Send_CMD_LowPower(void);
void activeControls (bool ask_distance, bool ask_consumption);

//Funcion principal - control del automata y el LCD
void thread__app_main_control (void *no_argument)
{
    uint32_t flags;
 
    //Variables locales para guardar el consumo y la hora en flash (se envian en la cola de mensajes junto al comando)
    float flash_consumo_tx; //revisar NAK buscar consumo_rx
    char flash_hora_tx[FLASH_NUM_CHAR_HORA];

    //Variables para guardar las medidas del consumo (y la hora) y mostrarlas en LCD tras obtenerse de flash
    float medidas_consumo[NUM_MAX_MUESTRA_CONSUMO];
    char horas_consumo[NUM_MAX_MUESTRA_CONSUMO][FLASH_NUM_CHAR_HORA];
    uint8_t numero_muestra = 0; //Numero de muestra actual a mostrar, se reinicia al entrar al modo de mostrar consumos

    //Variables para controlar el LCD en modo marcha atras
    lineas_distancia_t lineas_prev = LCD_LINE__NO_LINE;
    lineas_distancia_t lineas_actuales = LCD_LINE__NO_LINE;

    //Flags internas para el automata
    uint8_t state_enter = true;
    uint8_t state_error = false;
    
    //Inicializamos el LCD
    LCD_start();
    
    //Mostramos estado inicial de LEDs
    leds_activate_mask |= GET_MASK_LED(LED_GREEN);
    leds_activate_mask &= ~GET_MASK_LED(LED_BLUE);
    leds_activate_mask &= ~GET_MASK_LED(LED_RED);
    
    //Mostramos por el LCD que estamos en modo normal de funcionamiento
    LCD_write (LCD_LINE__ONE, "State: Normal");
    LCD_write (LCD_LINE__TWO, "Initializing...");

    state_enter = false;
    
    osDelay(7000);
    while(1)
    {
        flags = osThreadFlagsWait(FLAG__MAIN_CONTROL, osFlagsWaitAny, osWaitForever);
        
/************Flags comunes a todos los estados*************************************************/
        
        if (flags & FLAG__PRESS_UP) //Pulsaci�n arriba joystick
        {
            LCD_clean();
            app_state = (app_state == MAX_APP_STATE) ? FIRST_APP_STAGE : (app_state_t) (app_state + 1);
            state_enter = true;
        }
        
        //Se anade la hora y el consumo en la flash (dentro de flash se gestiona segun la muestra que sea en diferente sector)
        if (flags & FLAG__CONSUMO_EN_FLASH)
        {
            //Actualizamos las variables de envio
            flash_consumo_tx = (float) ((float) nRF_data_received_mando.consumo / 1000); //Guardamos el ultimo valor recibido desde el coche del consumo
            memcpy(flash_hora_tx, rtc_date_time[RTC_HOUR], FLASH_NUM_CHAR_HORA); //Guardamos el valor actual de la hora en el mensaje de envio hacia flash (HH:MM:SS, 8 char)
            //revisar debuggear que caracteres se meten, deben ser HH:MM:SS
            
            //Actualizamos Web revisar NAK unicamente actualizar cuando el valor cambie? revisar refresco de web
            sprintf(consumo_S, "%.2f", flash_consumo_tx);
            //revisar NAK mandar flag a web?�?

            //Anadimos en el mensaje de la cola los valores a a�adir y el comando de a�adir Consumo en flash
            flash_msg_data.command = FLASH_CMD__ADD_CONSUMPTION;
            flash_msg_data.consumption = &flash_consumo_tx;
            flash_msg_data.hour = flash_hora_tx;
            
            //Mandamos el mensaje
            if (osMessageQueuePut(id_flash_commands_queue, &flash_msg_data, NULL, 500) != osOK)
            {
                strncpy(detalleError, "MSG QUEUE ERROR FLASH", sizeof(detalleError) - 1);
                osThreadFlagsSet(id_thread__app_main, FLAG__ERROR);
            }
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
            LCD_write(LCD_LINE__TWO, detalleError); //detalleError es una variable global que se actualiza siempre que haya un error (prevalece el ultimo)
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
            Send_CMD_StateChange(app_state);
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
                        activeControls(false, true);    //desactivamos Distancia, activamos Consumo
                        
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
                        activeControls(false, true);    //desactivamos Distancia, activamos Consumo
                        
                        //Enviamos el estado al coche para habilitar/deshabilitar controles
                        Send_CMD_StateChange(app_state);
                        Send_CMD_LowPower();    //revisar Creo que no hace falta (ya se hace arriba)

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
                        
                        //Creamos el hilo de solicitud de distancia cada x tiempo que mandara comando de solicitud de distancia
                        activeControls(true, true);    //activamos Distancia, activamos Consumo
                        
                        state_enter = false;
                    }
                    
                    osDelay (1000); //Espera 1 segundo para visualizar que se entra al estado de marcha atras
                    if (flags & FLAG__MOSTRAR_DISTANCIA) //Flag enviado desde nRF TX tras recibir la distancia
                    {
                        //Se muestra la distancia por el LCD
                        if (lineas_prev != (lineas_actuales = calcularLineasDistancia(nRF_data_received_mando.distancia)))
                        {
                            LCD_mostrarLineasDistancia(lineas_actuales);
                            lineas_prev = lineas_actuales;
                        }
                        //Se pasa distancia por Web
                        sprintf(distancia_S,"%02d", nRF_data_received_mando.distancia);

                    }
                    break;
                    
                case APP_STAGE__MOSTRAR_CONSUMO:
                    if (state_enter)
                    {
                        //Pasamos un puntero hacia ambos arrays con todas las medidas de consumo y horas (las ultimas)
                        flash_msg_data.command = FLASH_CMD__GET_ALL_CONSUMPTION;
                        flash_msg_data.consumption = medidas_consumo;   //puntero a las medidas del consumo que se mostraran en lcd y flash
                        flash_msg_data.hour = &horas_consumo[0][0];     //puntero a las horas del consumo que se mostraran en lcd y flash
                        
                        //Cargar los datos de la flash
                        if (osMessageQueuePut(id_flash_commands_queue, &flash_msg_data, NULL, 500) != osOK)
                        {
                            strncpy(detalleError, "MSG QUEUE ERROR APP", sizeof(detalleError) - 1);
                            osThreadFlagsSet(id_thread__app_main, FLAG__ERROR);
                        }
                        
                        //Desactivamos todos los controles de otros estados
                        activeControls(false, true);    //desactivamos Distancia, activamos Consumo
                        
                        //Enviamos el estado al coche para habilitar/deshabilitar controles
                        Send_CMD_StateChange(app_state);
                        
                        //Mostramos estado inicial de LEDs
                        leds_activate_mask |= GET_MASK_LED(LED_GREEN);
                        leds_activate_mask &= ~GET_MASK_LED(LED_BLUE);
                        leds_activate_mask &= ~GET_MASK_LED(LED_RED);
                        
                        //Mostramos en la linea 1 del LCD que estamos en el modo de mostrar consumo
                        LCD_write (LCD_LINE__ONE, "State: Consumption    ");
                        
                        //Mostramos por LCD que se esta realizando la operacion de leer de memoria flash
                        LCD_write (LCD_LINE__TWO, "Reading FLASH...   ");
                        state_enter = false;
                    }

                    //Flag que se activara cuando los consumos esten listos para mostrar la primera muestra
                    if (flags & FLAG__CONSUMO_READY_FLASH)
                    {
                        //revisar NAK antes de pulsaba RIGHT
                        //Mostramos en la linea 1 del LCD que estamos en el modo de mostrar consumo
                        LCD_write (LCD_LINE__ONE, "State: Consumption    ");

                        //Reiniciamos la muestra seleccionada
                        numero_muestra = 0;

                        //Mostramos por el LCD la muestra indicada
                        LCD_mostrarConsumo(numero_muestra, medidas_consumo[numero_muestra]);
                    }

                    if (flags & FLAG__PRESS_RIGHT) //Mostramos el consumo de la flash en el LCD
                    {
                        //actualizamos el numero de la muestra que mostramos
                        numero_muestra = (numero_muestra == NUM_MAX_MUESTRA_CONSUMO - 1) ? 0 : numero_muestra + 1;
                        
                        //Mostramos por el LCD la muestra indicada
                        LCD_mostrarConsumo(numero_muestra, medidas_consumo[numero_muestra]);
                    }
                    
                    if (flags & FLAG__PRESS_LEFT) //Mostramos el consumo de la flash en el LCD
                    {
                        //actualizamos el numero de la muestra que mostramos
                        numero_muestra = (numero_muestra == 0) ? (NUM_MAX_MUESTRA_CONSUMO - 1) : numero_muestra - 1;
                        
                        //Mostramos por el LCD la muestra indicada
                        LCD_mostrarConsumo(numero_muestra, medidas_consumo[numero_muestra]);
                    }
                    break;
            }
        }
    }
}

void Init_AllAppThreads(void)
{
    
    /* Init all threads here. Nota: todos los hilos son de control. Luego ademas hay ficheros de driver que incluye cada hilo*/
    id_thread__app_main = osThreadNew(thread__app_main_control, NULL, NULL);
    if (id_thread__app_main == NULL)
    {
        //Error (no se puede gestionar como normalmente porque no hay app main)
        // strncpy(detalleError, "MSG QUEUE ERROR APP", sizeof(detalleError) - 1);
        // osThreadFlagsSet(id_thread__app_main, FLAG__ERROR);
        //SystemReset(); //revisar
    }
    netInitialize();            //Web
    //Init_RF_TX();               //Radiofrecuencia
    Init_LedsControl();         //Leds
    Init_RTC_Update();          //RTC
    Init_JoystickControl();     //Joystick
    //Init_VelocityCointrol();    //Velocidad / presion
    //Init_DirectionControl();    //Direccion / marchas
    Init_FlashControl();        //Flash
    
    
    //ESTADO INICIAL
    activeControls(false, true);    //desactivamos Distancia, activamos Consumo
}

//Funcion para activar/ desactivar los controles de pregunta/ respuesta del coche (Control de distancia y Control de consumo).
//En caso de que ya esten activos/ desactivados no se hace nada (gestion interna de creacion de threads)

void activeControls (bool ask_distance, bool ask_consumption)
{
    #ifndef RF_NO_ACTIVE
    if (ask_distance)
    {
        Init_askDistanceControl();
    }
    else
    {
        Stop_askDistanceControl();
    }
    if (ask_consumption)
    {
        Init_askConsumptionControl();
    }
    else
    {
        Stop_askConsumptionControl();
    }
    #endif
}

/**
 * @brief Calcula el número de líneas de advertencia que deben mostrarse en el display
 *        en función de la distancia detectada por un sensor.
 *
 *        Cuanto menor sea la distancia, más líneas se muestran (hasta un máximo de 3).
 *        Si el objeto está demasiado lejos, no se muestra ninguna línea.
 *        Si está demasiado cerca, se muestran las 3 líneas.
 *
 * @param distancia  Valor de distancia medido (0 a 500 en mm), donde 0 es muy cerca y 500 muy lejos.
 * @return lineas_distancia_t  Número de líneas a mostrar (0 a 3).
 */

lineas_distancia_t calcularLineasDistancia(uint16_t distancia)
{
    float tramo;
    uint32_t lineas;
    
    // Si la distancia es mayor o igual a 475 (muy lejos), no se muestra ninguna línea
    if (distancia >= (MAX_RANGE_DISTANCE - DISTANCE_SENSIBILITY)) //distancia >= (500-25)
        return LCD_LINE__NO_LINE;

    // Si la distancia es menor o igual a 25 (muy cerca), se muestran todas las líneas
    if (distancia <= (MIN_RANGE_DISTANCE + DISTANCE_SENSIBILITY)) //distancia <= (0+25)
        return LCD_MAX_LINES;

    // Se calcula el tamaño de cada tramo proporcional al número de líneas posibles
    // Ej: 500 / 3 = ~166.67 unidades por tramo
    tramo = (float) (MAX_RANGE_DISTANCE - MIN_RANGE_DISTANCE) / (LCD_MAX_LINES - LCD_MIN_LINES);
    
    // Se determina en qué tramo está la distancia (cuántas líneas "no" mostraría)
    lineas = ((distancia - MIN_RANGE_DISTANCE) / tramo); //Ej distancia = 300; lineas = (300-0)/166.67 = 1.79 = 1

    // Se invierte la relación para que menor distancia implique más líneas
    lineas = (LCD_MAX_LINES - lineas); //Ej: lineas = 3-1 = 2
    
    return (lineas_distancia_t)lineas;
}


//Utilizado para habilitar y deshabilitar controles en el coche (distancia)
void Send_CMD_StateChange (app_state_t app_state)
{
    #ifndef RF_NO_ACTIVE
    if (app_state == APP_STAGE__BACK_GEAR)  //Modo marcha atras. (empleado para saber el sentido de los servos y desactivar distancia)
    {
        nRF_data.command = nRF_CMD__BACK_GEAR_MODE;
    }
    else if (app_state == APP_STAGE__LOW_POWER) //Modo bajo consumo
    {
        nRF_data.command = nRF_CMD__LOW_POWER;
    }
    else    //Cualquier otro estado implica modo normal de funcionamiento en coche
    {
        nRF_data.command = nRF_CMD__NORMAL_MODE; //Modo normal de funcionamiento. (empleado para saber el sentido de los servos y desactivar distancia)
    }
    if (osMessageQueuePut(id_queue__nRF_TX_Data, &nRF_data, NULL, 1000) != osOK)
    {
        strncpy(detalleError, "MSG QUEUE ERROR RF", sizeof(detalleError) - 1);
        osThreadFlagsSet(id_thread__app_main, FLAG__ERROR);
    }
    #endif
}

void Send_CMD_LowPower(void) //revisar creo que se hace arriba
{
    #ifndef RF_NO_ACTIVE
    nRF_data.command = nRF_CMD__LOW_POWER;
    if (osMessageQueuePut(id_queue__nRF_TX_Data, &nRF_data, NULL, 1000) != osOK)
    {
        strncpy(detalleError, "MSG QUEUE ERROR RF", sizeof(detalleError) - 1);
        osThreadFlagsSet(id_thread__app_main, FLAG__ERROR);
    }
    #endif
}
