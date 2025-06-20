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
#include "errors.h"
#include "gpio.h"
#include "alarma_control.h"

#define NUM_MAX_MUESTRA_CONSUMO     10      //Buffer circular de consumo (memoria flash)
#define MAX_DISTANCE_DRIVER         9000    //A partir de aqui dara error (teoricamente 63490)
#define MAX_RANGE_DISTANCE          500     //Definir el valor en sensor que significa minima distancia
#define MIN_RANGE_DISTANCE          0       //Definir el valor en sensor que significa maxima distancia
#define DISTANCE_SENSIBILITY        25

//Variables globales
osThreadId_t id_thread__app_main;
char detalleError[LCD_MAX_CHARACTERS] = {0};
char moduloError[LCD_MAX_CHARACTERS] = {0};
app_state_t app_state = FIRST_APP_STAGE;

//Variables locales
//Variables para guardar las medidas del consumo (y la hora) y mostrarlas en LCD tras obtenerse de flash
float medidas_consumo[NUM_MAX_MUESTRA_CONSUMO];
char horas_consumo[NUM_MAX_MUESTRA_CONSUMO][FLASH_NUM_CHAR_HORA];
uint8_t numero_muestra = 0; //Numero de muestra actual a mostrar, se reinicia al entrar al modo de mostrar consumos

//Message to nRF queue
nRF_data_transmitted_t nRF_data_transmitted = {0};

//Message to flash queue (tanto para enviar como para recibir datos)
MSGQUEUE_FLASH_t flash_msg_data;

//Comunicacion con Web
char consumo_S [80];
char distancia_S[80];

//Funciones locales
lineas_distancia_t calcularLineasDistancia (uint16_t distancia);

//Funciones locales
void Send_CMD_StateChange (app_state_t app_state);
void activeControls (bool ask_distance, bool ask_consumption, bool alarma_control, bool direction_control, bool velocity_control);
void lcd_update_state(app_state_t app_state);

//Funcion principal - control del automata y el LCD
void thread__app_main_control (void *no_argument)
{
    uint32_t flags;
 
    //Variables locales para guardar el consumo y la hora en flash (se envian en la cola de mensajes junto al comando)
    float flash_consumo_tx;
    char flash_hora_tx[FLASH_NUM_CHAR_HORA];

    //Variables para controlar el LCD en modo marcha atras
    lineas_distancia_t lineas_prev = LCD_LINE__NO_LINE;
    lineas_distancia_t lineas_actuales = LCD_LINE__NO_LINE;

    //Flags internas para el automata
    bool state_enter = true;
    bool state_error = false;
    bool state_comms_rf_lost = false;
    bool state_magnet_lost = false;
    
    //Inicializamos el LCD
    LCD_start();
    
    app_state = FIRST_APP_STAGE;

    //Mostramos por el LCD que estamos inicializando el sistema
    LCD_write (LCD_LINE__ONE, "CARCOMM Project");
    LCD_write (LCD_LINE__TWO, "Initializing Device... 5");
    
    osDelay(1000);
    LCD_write (LCD_LINE__TWO, "Initializing Device... 4");
    osDelay(1000);
    LCD_write (LCD_LINE__TWO, "Initializing Device... 3");
    osDelay(1000);
    LCD_write (LCD_LINE__TWO, "Initializing Device... 2");
    osDelay(1000);
    LCD_write (LCD_LINE__TWO, "Initializing Device... 1");
    osDelay(1000);
    LCD_write (LCD_LINE__TWO, "Initializing Device... 0");
    osDelay(1000);
    
    while(1)
    {
        flags = osThreadFlagsWait(FLAG__MAIN_CONTROL, osFlagsWaitAny, osWaitForever);
        
/************Flags comunes de gestion de errores*************************************************/
        if (flags & FLAG__RF_LOST_COMMS_ERROR)
        {
            if (!state_comms_rf_lost && app_state != APP_STAGE__LOW_POWER)      //Si venimos de estado de comunicaciones Y no estamos en modo bajo consumo (siempre pierde comms)
            {
                //Activamos consumo para que al menos cada segundo el mando intente comunicar con el coche. El resto se desactiva ya que puede quedar pillado en algun estado molesto (alarma a tope)
                activeControls(false, true, false, true, true);    //desactivamos Distancia, activamos Consumo, desactivamos Alarma, desactivamos Direcion, desactivamos Velocidad

                //Estado de los leds de error
                leds_activate_mask |= GET_MASK_LED(LED_GREEN);
                leds_activate_mask |= GET_MASK_LED(LED_BLUE);
                leds_activate_mask |= GET_MASK_LED(LED_RED);

                //flag de error activo
                state_comms_rf_lost = true;

                //Actualizamos LCD con detalle de error
                LCD_write(LCD_LINE__ONE, moduloError);
                LCD_write(LCD_LINE__TWO, detalleError); //detalleError es una variable global que se actualiza siempre que haya un error (prevalece el ultimo)
            }
        }
        if (flags & (FLAG__DIR_MAG_NOT_PRES))       //Si hay perdida de comunicaciones
        {
            if (!state_magnet_lost)      //Si venimos de estado de comunicaciones
            {
                //Activamos consumo para que al menos cada segundo el mando intente comunicar con el coche. El resto se desactiva ya que puede quedar pillado en algun estado molesto (alarma a tope)
                activeControls(false, true, false, true, false);    //desactivamos Distancia, activamos Consumo, desactivamos Alarma, activamos Direcion, desactivamos Velocidad

                //Estado de los leds de error
                leds_activate_mask |= GET_MASK_LED(LED_GREEN);
                leds_activate_mask |= GET_MASK_LED(LED_BLUE);
                leds_activate_mask |= GET_MASK_LED(LED_RED);

                //flag de error activo
                state_magnet_lost = true;

                //Actualizamos LCD con detalle de error
                LCD_write(LCD_LINE__ONE, moduloError);
                LCD_write(LCD_LINE__TWO, detalleError); //detalleError es una variable global que se actualiza siempre que haya un error (prevalece el ultimo)
            }
        }
        
        if (flags & FLAG__RF_COMMS_RESTORED)
        {
            if (state_comms_rf_lost)   //Si venimos de estado de perdida de comunicaciones
            {
                //Los controles ya estan activados segun el estado en el que estemos (consumo siempre esta activo asi que siempre intenta comunicar)
                state_comms_rf_lost = false;       //Desactivamos flag de perdida de comunicaciones
                
                if (!state_magnet_lost)     //Si no hay error de iman
                {
                    state_enter = true;
                }
                else    //Si hay error de iman no detectado
                {
                    state_magnet_lost = false;      //Para que se trate el error de nuevo
                    push_error(MODULE__DIRECTION, ERR_CODE__MAGNET_NOT_PRESENT, 1);
                }
            } 
            else if (app_state == APP_STAGE__LOW_POWER)     //Si venimos de low power se restauran comunicaciones
            {
                app_state = APP_STATE__NORMAL;
                state_enter = true;
            }
        }

        if (flags & FLAG__DIR_MAG_DETECTED)
        {
            if (state_magnet_lost)   //Si venimos de estado de perdida de comunicaciones
            {
                //Los controles ya estan activados segun el estado en el que estemos (consumo siempre esta activo asi que siempre intenta comunicar)
                state_magnet_lost = false;       //Desactivamos flag de perdida de comunicaciones
                
                if (!state_comms_rf_lost)   //Si no hay error de RF
                {
                    state_enter = true;
                }
                else        //Si hay error de RF
                {
                    state_comms_rf_lost = false;        //Para que se trate el error de nuevo
                    push_error(MODULE__RF, ERR_CODE__RF_COMMS_LOST, 1);
                }
            } 
        }

        if (flags & FLAG__DRIVER_ERROR) //Flag de error de driver -> ERROR CATASTROFICO, se desactivan controles y no se permite aceptar el error. Solo se sale reiniciando. Aprende a programar si da este error
        {
            if (!state_error)       //Si venimos de estado de "no error"
            {
                //Desactivamos todos los controles
                activeControls(false, false, false, false, false);    //desactivamos Distancia, desactivamos Consumo, desactivamos Alarma
                
                //Mandamos al coche al estado bajo consumo para que consuma menos recursos.
                app_state = APP_STAGE__LOW_POWER;
                Send_CMD_StateChange(app_state);

                //Estado de los leds de error
                leds_activate_mask |= GET_MASK_LED(LED_GREEN);
                leds_activate_mask |= GET_MASK_LED(LED_BLUE);
                leds_activate_mask |= GET_MASK_LED(LED_RED);

                //flag de error activo
                state_error = true;

                //Actualizamos LCD con detalle de error
                LCD_write(LCD_LINE__ONE, moduloError);
                LCD_write(LCD_LINE__TWO, detalleError); //detalleError es una variable global que se actualiza siempre que haya un error (prevalece el ultimo)
            }
        }
         
/*****************************************************************************************************************************/
        
/************Comienzo de control de los estados*******************************************************************************/
        if ((!state_error) && (!state_comms_rf_lost) && (!state_magnet_lost))       //Solo se tiene en cuenta si no hay errores pendientes (tanto Driver como perdida de comunicaciones)
        {
            //Flags comunes a todos los estados
            if (flags & FLAG__PRESS_UP) //Pulsacion arriba joystick
            {
                if (app_state != APP_STAGE__LOW_POWER)
                {
                    app_state = (app_state == MAX_APP_STATE) ? FIRST_APP_STAGE : (app_state_t) (app_state + 1);
                    state_enter = true;
                }
            }

            if (flags & FLAG__PRESS_DOWN) //Pulsacion abajo joystick
            {
                app_state = (app_state <= FIRST_APP_STAGE) ? MAX_APP_STATE : (app_state_t) (app_state - 1);
                state_enter = true;
            }
            
            //Se anade la hora y el consumo en la flash (dentro de flash se gestiona segun la muestra que sea en diferente sector)
            if (flags & FLAG__CONSUMO_EN_FLASH)     
            {
                //Actualizamos las variables de envio
                flash_consumo_tx = (float) nRF_data_received_mando.consumo / 100.0f; //Guardamos el ultimo valor recibido desde el coche del consumo (el consumo se recibe como 1234 significando 12.34 mA)
                memcpy(flash_hora_tx, rtc_date_time[RTC_HOUR], FLASH_NUM_CHAR_HORA); //Guardamos el valor actual de la hora en el mensaje de envio hacia flash (HH:MM:SS, 8 char)
            
                //Actualizamos Web revisar NAK unicamente actualizar cuando el valor cambie? revisar refresco de web
                sprintf(consumo_S, "%.2f", flash_consumo_tx);
                //revisar NAK mandar flag a web??
            
                //Anadimos en el mensaje de la cola los valores a a嚙窮dir y el comando de a嚙窮dir Consumo en flash
                flash_msg_data.command = FLASH_CMD__ADD_CONSUMPTION;
                flash_msg_data.consumption = &flash_consumo_tx;
                flash_msg_data.hour = flash_hora_tx;
                
                //Mandamos el mensaje
                if (osMessageQueuePut(id_flash_commands_queue, &flash_msg_data, NULL, DRIVER_TIME_WAIT) != osOK)
                {
                    push_error(ERR_CODE__QUEUE, MODULE__FLASH, 0);
                }
            }
            
            if (flags & FLAG__ENTER_LOW_POWER)  //Entramos/salimos del modo bajo consumo (se entra con el pulsador azul)
            {
                //app_state = (app_state == APP_STAGE__LOW_POWER) ? APP_STATE__NORMAL : APP_STAGE__LOW_POWER;
                app_state = APP_STAGE__LOW_POWER;
                state_enter = true;
            }
            
            //Automata de control
            switch (app_state)
            {
                case APP_STATE__NORMAL:               
                    if (state_enter)
                    {
                        LCD_clean();
                        //Desactivamos todos los controles de otros estados
                        activeControls(false, true, false, true, true);    //desactivamos Distancia, activamos Consumo, desactivamos Alarma
                        
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
                        //Mostramos la hora en el LCD -linea 2
                        LCD_write (LCD_LINE__TWO, rtc_date_time[RTC_HOUR]);
                    }
                    //En caso de error, se muestra en LCD
                    break;
                    
                case APP_STAGE__LOW_POWER:               
                    if (state_enter)
                    {
                        LCD_clean();
                        //Desactivamos todos los controles de otros estados
                        activeControls(false, true, false, false, false);    //desactivamos Distancia, activamos Consumo, desactivamos Alarma, desactivamos Direcion, desactivamos Velocidad
                        
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
                        LCD_clean();
                        //Enviamos el estado al coche para habilitar/deshabilitar controles
                        Send_CMD_StateChange(app_state);
                        
                        //Mostramos estado inicial de LEDs
                        leds_activate_mask &= ~GET_MASK_LED(LED_GREEN);
                        leds_activate_mask &= ~GET_MASK_LED(LED_BLUE);
                        leds_activate_mask |= GET_MASK_LED(LED_RED);
                        
                        //Creamos el hilo de solicitud de distancia cada x tiempo que mandara comando de solicitud de distancia
                        activeControls(true, true, true, true, true);    //activamos Distancia, activamos Consumo, activamos Alarma

                        //Mostramos por el LCD que estamos en modo de bajo consumo
                        LCD_write (LCD_LINE__ONE, "State: Back Gear");
                        
                        //Espera 3 segundo para visualizar que se entra al estado de marcha atras
                        osDelay(3000); 
                        
                        //Motramos la primera iteraccion (porque cuando volvemos de error, hasta que no cambien no se hace)
                        lineas_actuales = calcularLineasDistancia(nRF_data_received_mando.distancia);
                        LCD_mostrarLineasDistancia(lineas_actuales);
                        
                        state_enter = false;
                    }
                    
                    if (flags & FLAG__MOSTRAR_DISTANCIA) //Flag enviado desde nRF TX tras recibir la distancia
                    {
                        if (nRF_data_received_mando.distancia >= MAX_DISTANCE_DRIVER) //Esta limitado por driver para pintar en LCD, por lo que si llega mas de 500 se ponen 0 lineas y ya
                        {
                            push_error(MODULE__ASK_DISTANCE, ERR_CODE__DATA_CORRUPT, 0);  //Revisar Se envia pero no se gestiona
                        }
                        else
                        {
                            //Se muestra la distancia por el LCD
                            if (lineas_prev != (lineas_actuales = calcularLineasDistancia(nRF_data_received_mando.distancia)))
                            {
                                LCD_mostrarLineasDistancia(lineas_actuales);
                                osThreadFlagsSet(id_thread__AlarmaControl, SET_ALARM_FLAG(lineas_actuales));
                                lineas_prev = lineas_actuales;
                            }
                            //Se pasa distancia por Web
                            sprintf(distancia_S,"%02d", nRF_data_received_mando.distancia);
                        }

                    }
                    break;
                    
                case APP_STAGE__MOSTRAR_CONSUMO:
                    if (state_enter)
                    {
                        LCD_clean();
                        //Pasamos un puntero hacia ambos arrays con todas las medidas de consumo y horas (las ultimas)
                        flash_msg_data.command = FLASH_CMD__GET_ALL_CONSUMPTION;
                        flash_msg_data.consumption = medidas_consumo;   //puntero a las medidas del consumo de flash que se mostraran en lcd y web
                        flash_msg_data.hour = &horas_consumo[0][0];     //puntero a las horas del consumo de flash que se mostraran en lcd y web
                        
                        //Cargar los datos de la flash
                        if (osMessageQueuePut(id_flash_commands_queue, &flash_msg_data, NULL, DRIVER_TIME_WAIT) != osOK)
                        {
                            push_error(MODULE__APP, ERR_CODE__QUEUE, 0);
                        }
                        
                        //Desactivamos todos los controles de otros estados
                        activeControls(false, false, false, true, true);    //desactivamos Distancia, activamos Consumo, desactivamos Alarma
                        
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
                        //Reiniciamos la muestra seleccionada
                        numero_muestra = 0;

                        //Mostramos por el LCD la muestra indicada - linea 2
                        LCD_mostrarConsumo(numero_muestra, medidas_consumo[numero_muestra]);
                    }

                    if (flags & FLAG__PRESS_RIGHT) //Mostramos el consumo de la flash en el LCD
                    {
                        //actualizamos el numero de la muestra que mostramos
                        numero_muestra = (numero_muestra == NUM_MAX_MUESTRA_CONSUMO - 1) ? 0 : numero_muestra + 1;
                        
                        //Mostramos por el LCD la muestra indicada - linea 2
                        LCD_mostrarConsumo(numero_muestra, medidas_consumo[numero_muestra]);
                    }
                    
                    if (flags & FLAG__PRESS_LEFT) //Mostramos el consumo de la flash en el LCD
                    {
                        //actualizamos el numero de la muestra que mostramos
                        numero_muestra = (numero_muestra == 0) ? (NUM_MAX_MUESTRA_CONSUMO - 1) : numero_muestra - 1;
                        
                        //Mostramos por el LCD la muestra indicada - linea 2
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
    Init_LedsControl();         //Leds
    Init_RTC_Update();          //RTC
    Init_JoystickControl();     //Joystick
    Init_RF_TX();               //Radiofrecuencia
    Init_VelocityControl();    //Velocidad / presion
    Init_DirectionControl();    //Direccion / marchas
    init_pulsador();            //Pulsador azul
    Init_FlashControl();        //Flash
    
    //ESTADO INICIAL
    activeControls(false, true, false, true, true);    //desactivamos Distancia, activamos Consumo, desactivamos Alarma
}

//Funcion para activar/ desactivar los controles de pregunta/ respuesta del coche (Control de distancia y Control de consumo).
//En caso de que ya esten activos/ desactivados no se hace nada (gestion interna de creacion de threads)

void activeControls (bool ask_distance, bool ask_consumption, bool alarma_control, bool direction_control, bool velocity_control)
{
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
    if (alarma_control)
    {
        Init_AlarmaControl();
    }
    else
    {
        Deinit_AlarmaControl();
    }
    if (direction_control)
    {
        Init_DirectionControl();
    }
    else
    {
        DeInit_DirectionControl();
    }
    if (velocity_control)
    {
        Init_VelocityControl();
    }
    else
    {
        DeInit_VelocityControl();
    }
}

//Gestion del LCD tras entrar a un estado - es buena idea pero no lo
void lcd_update_state(app_state_t app_state)
{
    switch (app_state)
    {
        case APP_STATE__NORMAL:
            LCD_write (LCD_LINE__ONE, "State: Normal");
            break;
        case APP_STAGE__LOW_POWER:
            LCD_write (LCD_LINE__ONE, "State: Low Power");
            break;
        case APP_STAGE__BACK_GEAR:
            LCD_write (LCD_LINE__ONE, "State: Back Gear");
            osDelay(1000); //Espera 1 segundo para visualizar que se entra al estado de marcha atras
            break;
        case APP_STAGE__MOSTRAR_CONSUMO: //Lo mas seguro es que flash ya se haya leido si es que da error
            //Mostramos en la linea 1 del LCD que estamos en el modo de mostrar consumo
            LCD_write (LCD_LINE__ONE, "State: Consumption    ");
            //Reiniciamos la muestra seleccionada
            numero_muestra = 0;
            //Mostramos por el LCD la muestra indicada
            LCD_mostrarConsumo(numero_muestra, medidas_consumo[numero_muestra]);
            break;
    }
}

/**
 * @brief Calcula el numero de li要eas de advertencia que deben mostrarse en el display
 *        en funcion de la distancia detectada por un sensor.
 *
 *        Cuanto menor sea la distancia, mas li要eas se muestran (hasta un maximo de 3).
 *        Si el objeto esta demasiado lejos, no se muestra ninguna li要ea.
 *        Si esta demasiado cerca, se muestran las 3 li要eas.
 *
 * @param distancia  Valor de distancia medido (0 a 500 en mm), donde 0 es muy cerca y 500 muy lejos.
 * @return lineas_distancia_t  Numero de lineas a mostrar (0 a 3).
 */

lineas_distancia_t calcularLineasDistancia(uint16_t distancia)
{
    float tramo;
    uint32_t lineas;
    
    // Si la distancia es mayor o igual a 475 (muy lejos), no se muestra ninguna linea
    if (distancia >= (MAX_RANGE_DISTANCE - DISTANCE_SENSIBILITY)) //distancia >= (500-25)
        return LCD_LINE__NO_LINE;

    // Si la distancia es menor o igual a 25 (muy cerca), se muestran todas las lineas
    if (distancia <= (MIN_RANGE_DISTANCE + DISTANCE_SENSIBILITY)) //distancia <= (0+25)
        return LCD_MAX_LINES;

    // Se calcula el tamano de cada tramo proporcional al numero de lineas posibles
    // Ej: 500 / 3 = ~166.67 unidades por tramo
    tramo = (float) (MAX_RANGE_DISTANCE - MIN_RANGE_DISTANCE) / (LCD_MAX_LINES - LCD_MIN_LINES);
    
    // Se determina en que tramo esta la distancia (cuantas lineas "no" mostrara)
    lineas = ((distancia - MIN_RANGE_DISTANCE) / tramo); //Ej distancia = 300; lineas = (300-0)/166.67 = 1.79 = 1

    // Se invierte la relacion para que menor distancia implique mas lineas
    lineas = (LCD_MAX_LINES - lineas); //Ej: lineas = 3-1 = 2
    
    return (lineas_distancia_t)lineas;
}


//Utilizado para habilitar y deshabilitar controles en el coche (distancia, bajo consumo)
void Send_CMD_StateChange (app_state_t app_state)
{
    if (app_state == APP_STAGE__BACK_GEAR)  //Modo marcha atras. (empleado para saber el sentido de los servos y desactivar distancia)
    {
        nRF_data_transmitted.command = nRF_CMD__BACK_GEAR_MODE;
    }
    else if (app_state == APP_STAGE__LOW_POWER) //Modo bajo consumo
    {
        nRF_data_transmitted.command = nRF_CMD__LOW_POWER;
    }
    else    //Cualquier otro estado implica modo normal de funcionamiento en coche
    {
        nRF_data_transmitted.command = nRF_CMD__NORMAL_MODE; //Modo normal de funcionamiento. (empleado para saber el sentido de los servos y desactivar distancia)
    }
    if (osMessageQueuePut(id_queue__nRF_TX_Data, &nRF_data_transmitted, NULL, DRIVER_TIME_WAIT) != osOK)
    {
        push_error(MODULE__APP, ERR_CODE__QUEUE, (uint8_t) nRF_data_transmitted.command);
    }
}

