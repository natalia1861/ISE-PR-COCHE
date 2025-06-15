#include "cmsis_os2.h"                          // CMSIS RTOS header file
#include "Driver_I2C.h"
#include "stm32f4xx_hal.h"
#include "sensor_AS5600.h"
#include <stdio.h>
#include <math.h> 
#include "direction_control.h"
#include "errors.h"

/**
 * Este sensor puede comenzar a operar con sus valores de configuración por defecto,
 * por lo que la lectura del registro CONF (0x07–0x08) es opcional.
 *
 * - El AS5600 inicia en modo normal con filtros y rangos preconfigurados.
 * - No es necesario configurar CONF para obtener lecturas válidas de ángulo.
 * - Sin embargo, puede leerse para diagnóstico, depuración o personalización:
 *     - Filtrado de salida
 *     - Histéresis
 *     - Modos de energía
 *
 */

#define I2C_TX_RX                   0x0CU   //Raw angle
#define AS5600_I2C_ADDRESS_SLAVE    0x36    // Direccion I2C del AS560

//Registers
#define AS5600_STATUS_REG           0x0B    // Registro de estado
#define AS5600_RAW_ANGLE            0x0C    // Raw angle (2 bytes)
#define AS5600_CONFIG_REG           0x07    // Configuration register (2 byte)

//Default configuration
#define AS5600_CONFIG_DEFAULT       0x00    //Configuracion por defecto del sensor (que es la misma que queremos)
//Segun el datasheet hay que leer la configuracion y en caso de querer cambiarla, se debe actualizar. 
//Como en nuestro caso, la configuracion es siempre la misma y es la de por defecto. Omitimos este paso

#define AS5600_ZPOS_HIGH_REG        0x01
#define AS5600_ZPOS_LOW_REG         0x00

#define FULL_ROTATION_STEPS_360_GRADOS   4096
#define HALF_ROTATION_STEPS_180_GRADOS (FULL_ROTATION_STEPS_360_GRADOS / 2)
#define INICIAL_0_GRADOS            0

#define IDEAL      0 //Implica que en la primera lectura justo el angulo medido es de 180 (comienza justo en la mitad) - no va a ocurrir
//#define PEQUENO    1
//#define GRANDE     2
#define IRREGULAR  3 //El primer angulo medido de referencia sera diferente a 180 por lo que tendremos que guardar un offset inicial que se usara para todas las medidas posteriores

//#define CENTRO     0
//#define IZQUIERDA  1
//#define DERECHA    2

#define GIRO_IZQUIERDA      0
#define GIRO_DERECHA        360

//Para el monitoreo de las vueltas
#define UMBRAL_BAJO_GRADOS 100
#define UMBRAL_ALTO_GRADOS 300

//Thread
osThreadId_t id_sensor_AS5600;                        // thread id
void sensor_AS5600_Thread(void *argument);            // thread function
osMessageQueueId_t id_volante_MsgQueue;

//I2C (1)
extern ARM_DRIVER_I2C Driver_I2C1;
static ARM_DRIVER_I2C *I2Cdrv = &Driver_I2C1;

//Funciones internas
void I2C_TX_RX_Callback(uint32_t flags);
bool as5600_isMagnetDetected(void);       //No se utiliza - revisar NAK

//Variables globales para el control de angulo y vueltas
uint16_t angle_offset = 0; //valor que toma como inicial y lo inicia a 180�
uint16_t angle_raw = 0; //valor nuevo que coge del sensor

uint16_t last_angle = 0; //Anterior valore leido respecto al offset corregido
uint16_t angle_new = 0;// Angulo respecto al offset corregido
int8_t vuelta = 0;
bool start = false;

float angle_deg = 0;//Valor que devuelve la funci�n de que grados gira
float angle_send = 0;

//Para saber donde se encuentra 0 grados y 360 grados respecto al offset
uint16_t MAX_GIRO_DERECHA = 0;
uint16_t MAX_GIRO_IZQUIERDA = 0;

//PRUEBAS
uint8_t modo = IDEAL;

AS5600_status_t as5600_init(void)
{
    I2Cdrv-> Initialize     (I2C_TX_RX_Callback);
    I2Cdrv-> PowerControl   (ARM_POWER_FULL);
    I2Cdrv-> Control        (ARM_I2C_BUS_SPEED, ARM_I2C_BUS_SPEED_STANDARD);
    I2Cdrv-> Control        (ARM_I2C_BUS_CLEAR, 0);

    // Verificamos si el imán está presente antes de iniciar
    if (!as5600_isMagnetDetected()) {
        return AS5600_MAGNET_NOT_DETECTED;  // Puedes definir este código como -1 o el valor que prefieras
    }
  
    return AS5600_OK;
}

AS5600_status_t as5600_readout(float* read_angle){

    uint32_t flags;
    uint8_t reg = I2C_TX_RX;
    uint8_t content_rx[2];
  
    //Para el primer valor
    uint16_t rest = 0;
    uint16_t angle_first = 0;

    // Verificamos si el imán está presente antes de iniciar
    if (!as5600_isMagnetDetected()) {
        return AS5600_MAGNET_NOT_DETECTED;  // Puedes definir este código como -1 o el valor que prefieras
    }
    
    //First data is trash
    if (!start)
    {
      // Enviar direccion de lectura
      I2Cdrv->MasterTransmit(AS5600_I2C_ADDRESS_SLAVE, &reg, 1, true);
      flags = osThreadFlagsWait(I2C_TX_RX, osFlagsWaitAny, DRIVER_TIME_WAIT);

      // Recibir datos
      I2Cdrv->MasterReceive(AS5600_I2C_ADDRESS_SLAVE, content_rx, 2, false);
      flags = osThreadFlagsWait(I2C_TX_RX, osFlagsWaitAny, DRIVER_TIME_WAIT);
    }
    
    // Se envia comando de lectura de angulo raw
    I2Cdrv->MasterTransmit(AS5600_I2C_ADDRESS_SLAVE, &reg, 1, true);
    flags = osThreadFlagsWait(I2C_TX_RX, osFlagsWaitAny, DRIVER_TIME_WAIT);

    // Recibir datos
    I2Cdrv->MasterReceive(AS5600_I2C_ADDRESS_SLAVE, content_rx, 2, false);
    flags = osThreadFlagsWait(I2C_TX_RX, osFlagsWaitAny, DRIVER_TIME_WAIT);

    //Angulo ente 0 y 4095
    angle_raw = (content_rx[0] << 8) | content_rx[1];

    if (!start)
    { //Hago que el offset sea 2048 que es 180 grados (solo lo hace con el primer valor que recibe)
      if (angle_raw < HALF_ROTATION_STEPS_180_GRADOS)
      {
        rest = HALF_ROTATION_STEPS_180_GRADOS - angle_raw;
        angle_first = angle_raw + rest;
      }else if(angle_raw > HALF_ROTATION_STEPS_180_GRADOS)
      {
        rest = angle_raw - HALF_ROTATION_STEPS_180_GRADOS;
        angle_first = angle_raw - rest;
      }else 
      {
        angle_first = angle_raw;
      }
      angle_offset = angle_raw;
      start = true;
      
      if (HALF_ROTATION_STEPS_180_GRADOS > angle_offset || HALF_ROTATION_STEPS_180_GRADOS < angle_offset)
      {//si el valor es offset es menor o mayor que 2048 (180 grados)
        modo = IRREGULAR;
      }else { //si el offset justo es 2048 es decir 180 grados
        modo = IDEAL;
      }
      
      angle_deg = (angle_first * 360.0f) / FULL_ROTATION_STEPS_360_GRADOS;
      //printf("%.2f\n",angle_deg);
      
      last_angle = angle_deg;
      
      (*read_angle) = angle_deg;
      return AS5600_OK;
    }
    
    /*               Si va a la derecha aumentan los grados                   */
    /*             Si va a la izquierda disminuyen los grados                 */
    
    /*                  HAY QUE MODIFICAR LA SEGUNDA TERCERA ... MEDIDA                 */
    /*                    TENIENDO EN CUENTA EL OFFSET DEL PRINCIPIO                    */

    //Corrigo el valor referenciado al offset
    if (modo == IDEAL)
    { // Si el offset esta en el medio de 180 grados que equivale a 2048
      angle_new = angle_raw;
    } else if (modo == IRREGULAR){ // Si el offset esta cerca del 0 o de 360
      angle_new = (HALF_ROTATION_STEPS_180_GRADOS - angle_offset + angle_raw) & 0x0FFF;
    }

    //Lo comparo con los grados cogidos (el actual y el anterior)
    angle_deg = (angle_new * 360.0f) / FULL_ROTATION_STEPS_360_GRADOS;
    if ((last_angle > UMBRAL_ALTO_GRADOS) && (angle_deg < UMBRAL_BAJO_GRADOS)) 
    {
        vuelta++;  // Giro completo horario (paso por 0 grados)
    } else if ((last_angle < UMBRAL_BAJO_GRADOS) && (angle_deg > UMBRAL_ALTO_GRADOS)) 
    {
        vuelta--;  // Giro completo antihorario (paso por 4095, 360 grados)
    }
    
    last_angle = angle_deg;
    
    if (vuelta <= -1) //Si hace una vuelta a la izquierda envia una constante hasta que las vueltas sean 0
    {
      angle_deg = GIRO_IZQUIERDA;
    }else if (vuelta >= 1) //Si hace una vuelta a la derecha envia una constante hasta que las vueltas sean 0
    {
      angle_deg = GIRO_DERECHA;
    }

    //printf("GRADOS:%.2f  VUELTAS:%d\n",angle_deg , vuelta);
    
    (*read_angle) = angle_deg;
    return AS5600_OK;
}

void I2C_TX_RX_Callback(uint32_t flags)
{
  if((flags & ARM_I2C_EVENT_TRANSFER_DONE) == true){
    osThreadFlagsSet(id_thread__DirectionControl, I2C_TX_RX);
  }
  if((flags & ARM_I2C_EVENT_SLAVE_TRANSMIT) == true){
    osThreadFlagsSet(id_thread__DirectionControl, I2C_TX_RX);
  }
  if((flags & ARM_I2C_EVENT_SLAVE_RECEIVE) == true){
    osThreadFlagsSet(id_thread__DirectionControl, I2C_TX_RX);
  }
}

/**
 * @brief Comprueba si el imán está presente frente al sensor AS5600.
 *
 * El AS5600 puede devolver lecturas de ángulo aunque el imán no esté presente.
 * Este método revisa el bit 5 (MD - Magnet Detected) del registro de estado (0x0B)
 * para confirmar que el imán está correctamente posicionado.
 *
 * @return true si el imán está presente, false en caso contrario o error de comunicación.
 */
bool as5600_isMagnetDetected(void)
{
    uint8_t reg = AS5600_STATUS_REG;      // Dirección del registro STATUS (0x0B)
    uint8_t status = 0;
    uint32_t flags;

    // Escribir la dirección del registro que se desea leer
    if (I2Cdrv->MasterTransmit(AS5600_I2C_ADDRESS_SLAVE, &reg, 1, true) != ARM_DRIVER_OK)
        return false;

    // Esperar a que termine la transmisión
    flags = osThreadFlagsWait(I2C_TX_RX, osFlagsWaitAny, DRIVER_TIME_WAIT);
    if (!(flags & I2C_TX_RX))
        return false;

    // Leer el contenido del registro STATUS
    if (I2Cdrv->MasterReceive(AS5600_I2C_ADDRESS_SLAVE, &status, 1, false) != ARM_DRIVER_OK)
        return false;

    // Esperar a que termine la recepción
    flags = osThreadFlagsWait(I2C_TX_RX, osFlagsWaitAny, DRIVER_TIME_WAIT);
    if (!(flags & I2C_TX_RX))
        return false;

    // Bit 5 indica si el imán está detectado (MD = 1)
    return (status & (GET_BIT_MASK(MD_BIT))) != 0;
}
