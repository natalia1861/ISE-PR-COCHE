#include "cmsis_os2.h"                          // CMSIS RTOS header file
#include "Driver_I2C.h"
#include "stm32f4xx_hal.h"
#include "sensor_AS5600.h"
#include <stdio.h>
#include <math.h> 

#define I2C_TX_RX           0x0CU
#define AS5600_I2C_ADDRESS_SLAVE 0x36 // Dirección I2C del AS560
#define AS5600_STATUS_REG   0x0B  // Registro de estado

#define AS5600_ZPOS_HIGH_REG  0x01
#define AS5600_ZPOS_LOW_REG   0x00

#define FULL_ROTATION_STEPS_360_GRADOS   4096
#define HALF_ROTATION_STEPS_180_GRADOS (FULL_ROTATION_STEPS_360_GRADOS / 2)
#define INICIAL_0_GRADOS    0

#define IDEAL      0
//#define PEQUENO    1
//#define GRANDE     2
#define IRREGULAR  3

//#define CENTRO     0
//#define IZQUIERDA  1
//#define DERECHA    2

#define GIRO_IZQUIERDA 0
#define GIRO_DERECHA   360

//#define DIFERENCIA  200

osThreadId_t id_sensor_AS5600;                        // thread id
void sensor_AS5600_Thread(void *argument);                   // thread function

osMessageQueueId_t id_volante_MsgQueue;

//I2C (1)
extern ARM_DRIVER_I2C Driver_I2C1;
static ARM_DRIVER_I2C *I2Cdrv = &Driver_I2C1;

int as5600_init(void);
void I2C_TX_RX_Callback(uint32_t flags);
float as5600_readout(void);
int as5600_setZPOS_180(void);
bool as5600_isMagnetDetected(void);

uint16_t angle_offset = 0; //valor que toma como inicial y lo inicia a 180º
uint16_t angle_raw = 0; //valor nuevo que coge del sensor

uint16_t last_angle = 0; //Anterior valore leido respecto al offset corregido
uint16_t angle_new = 0;// Angulo respecto al offset corregido
int8_t vuelta = 0;

float angle_deg = 0;//Valor que devuelve la función de que grados gira
float angle_send = 0;

//Para saber donde se encuentra 0º y 360º respecto al offset
uint16_t MAX_GIRO_DERECHA = 0;
uint16_t MAX_GIRO_IZQUIERDA = 0;

//PRUEBAS
uint8_t modo = IDEAL;
//uint16_t giro = CENTRO;

uint8_t status_sen = 0;

int Init_AS5600_Thread(void)
{
    id_volante_MsgQueue = osMessageQueueNew(1, sizeof(angle_send), NULL);
    id_sensor_AS5600 = osThreadNew(sensor_AS5600_Thread, NULL, NULL);
    if (id_sensor_AS5600 == NULL) {
        return(-1);
    }
    return(0);
}

void sensor_AS5600_Thread (void *argument)
{
  as5600_init();
  as5600_setZPOS_180();
  while(1){
    osDelay(100);
    angle_send = as5600_readout();
    osMessageQueuePut(id_volante_MsgQueue, &angle_send, 0U, 0U);
//    if(as5600_isMagnetDetected()){
//      printf ("SII \n");
//    }else{
//      printf ("NOO \n");
//    }
  }
}

int as5600_init(void)
{
    I2Cdrv-> Initialize     (I2C_TX_RX_Callback);
    I2Cdrv-> PowerControl   (ARM_POWER_FULL);
    I2Cdrv-> Control        (ARM_I2C_BUS_SPEED, ARM_I2C_BUS_SPEED_STANDARD);
    I2Cdrv-> Control        (ARM_I2C_BUS_CLEAR, 0);

    uint8_t content[2];
    uint32_t flags;

    I2Cdrv->MasterReceive(AS5600_I2C_ADDRESS_SLAVE, &content[1], 1, false);
    flags = osThreadFlagsWait(I2C_TX_RX, osFlagsWaitAny, osWaitForever);
  
    return 0;
}

float as5600_readout(void){

    uint32_t flags;
    uint8_t reg = I2C_TX_RX;
    uint8_t content_rx[2];
  
    //Para el primer valor
    uint16_t rest = 0;
    uint16_t angle_first = 0;
  
    static bool start = false;

    if (!start){//HAGO ESTO PORQUE EL PRIMER VALOR QUE ME DA NO TIENE QUE VER CON GRADOS
      // Enviar dirección de lectura
      I2Cdrv->MasterTransmit(AS5600_I2C_ADDRESS_SLAVE, &reg, 1, true);
      flags = osThreadFlagsWait(I2C_TX_RX, osFlagsWaitAny, osWaitForever);

      // Recibir datos
      I2Cdrv->MasterReceive(AS5600_I2C_ADDRESS_SLAVE, content_rx, 2, false);
      flags = osThreadFlagsWait(I2C_TX_RX, osFlagsWaitAny, osWaitForever);
    }
    
    // Enviar dirección de lectura
    I2Cdrv->MasterTransmit(AS5600_I2C_ADDRESS_SLAVE, &reg, 1, true);
    flags = osThreadFlagsWait(I2C_TX_RX, osFlagsWaitAny, osWaitForever);

    // Recibir datos
    I2Cdrv->MasterReceive(AS5600_I2C_ADDRESS_SLAVE, content_rx, 2, false);
    flags = osThreadFlagsWait(I2C_TX_RX, osFlagsWaitAny, osWaitForever);

    angle_raw = (content_rx[0] << 8) | content_rx[1];

    if (!start){ //Hago que el offset sea 2048 que es 180º (solo lo hace con el primer valor que recibe)
      if (angle_raw < HALF_ROTATION_STEPS_180_GRADOS){
        rest = HALF_ROTATION_STEPS_180_GRADOS - angle_raw;
        angle_first = angle_raw + rest;
      }else if(angle_raw > HALF_ROTATION_STEPS_180_GRADOS){
        rest = angle_raw - HALF_ROTATION_STEPS_180_GRADOS;
        angle_first = angle_raw - rest;
      }else {
        angle_first = angle_raw;
      }
      angle_offset = angle_raw;
      start = true;
      
      if (HALF_ROTATION_STEPS_180_GRADOS > angle_offset || HALF_ROTATION_STEPS_180_GRADOS < angle_offset){//si el valor es offset es menor o mayor que 2048 (180º)
        modo = IRREGULAR;
      }else { //si el offset justo es 2048 es decir 180º //DUDO MUCHO QUE PASE PERO LO PONGO
        modo = IDEAL;
      }
      
      angle_deg = (angle_first * 360.0f) / FULL_ROTATION_STEPS_360_GRADOS;
      printf("%.2f\n",angle_deg);
      
      last_angle = angle_deg;
      
      return angle_deg;
    }
    
    /*               Si va a la derecha aumentan los grados                   */
    /*             Si va a la izquierda disminuyen los grados                 */
    
    /*                  HAY QUE MODIFICAR LA SEGUNDA TERCERA ... MEDIDA                 */
    /*                    TENIENDO EN CUENTA EL OFFSET DEL PRINCIPIO                    */
          //Corrigo el valor referenciado al offset
    if (modo == IDEAL){ // Si el offset esta en el medio de 180º que equivale a 2048
      angle_new = angle_raw;
    } else if (modo == IRREGULAR){ // Si el offset esta cerca del 0º o de 360º
      angle_new = (HALF_ROTATION_STEPS_180_GRADOS - angle_offset + angle_raw) & 0x0FFF;
    }
    
    //Para el monitoreo de las vueltas  SE RAYA NO SE COMO HACERLO
    #define UMBRAL_BAJO_GRADOS 100
    #define UMBRAL_ALTO_GRADOS 300

    //Lo comparo con los grados cogidos (el actual y el anterior)
    angle_deg = (angle_new * 360.0f) / FULL_ROTATION_STEPS_360_GRADOS;
    if ((last_angle > UMBRAL_ALTO_GRADOS) && (angle_deg < UMBRAL_BAJO_GRADOS)) {
        vuelta++;  // Giro completo horario (paso por 0°)
    } else if ((last_angle < UMBRAL_BAJO_GRADOS) && (angle_deg > UMBRAL_ALTO_GRADOS)) {
        vuelta--;  // Giro completo antihorario (paso por 4095, 360º)
    }
    
    last_angle = angle_deg;
    
    if (vuelta <= -1){//Si hace una vuelta a la izquierda envia una constante hasta que las vueltas sean 0
      angle_deg = GIRO_IZQUIERDA;
    }else if (vuelta >= 1){//Si hace una vuelta a la derecha envia una constante hasta que las vueltas sean 0
      angle_deg = GIRO_DERECHA;
    }

    printf("GRADOS:%.2f  VUELTAS:%d\n",angle_deg , vuelta);
    
    return angle_deg;
}

void I2C_TX_RX_Callback(uint32_t flags)
{
  if((flags & ARM_I2C_EVENT_TRANSFER_DONE) == true){
    osThreadFlagsSet(id_sensor_AS5600, I2C_TX_RX);
  }
  if((flags & ARM_I2C_EVENT_SLAVE_TRANSMIT) == true){
    osThreadFlagsSet(id_sensor_AS5600, I2C_TX_RX);
  }
  if((flags & ARM_I2C_EVENT_SLAVE_RECEIVE) == true){
    osThreadFlagsSet(id_sensor_AS5600, I2C_TX_RX);
  }
}

/*           NO USO ESTAS FUNCIONES           */
// --- Configurar el offset ZPOS = 2048 (180°) ---
int as5600_setZPOS_180(void) 
{
    uint8_t tx_buffer[3];
    tx_buffer[0] = AS5600_ZPOS_HIGH_REG;             // Dirección inicial
    tx_buffer[1] = (HALF_ROTATION_STEPS_180_GRADOS >> 8) & 0x0F;             // 4 bits altos
    tx_buffer[2] = HALF_ROTATION_STEPS_180_GRADOS & 0xFF;                    // 8 bits bajos

    if (I2Cdrv->MasterTransmit(AS5600_I2C_ADDRESS_SLAVE << 1, tx_buffer, 3, false) != ARM_DRIVER_OK) {
        return -1; // Error de transmisión
    }

    osDelay(10); // Espera por seguridad

    return 0;
}

bool as5600_isMagnetDetected(void) 
{
    uint8_t reg = AS5600_STATUS_REG;

    // Escribir la dirección del registro que queremos leer
    if (I2Cdrv->MasterTransmit(AS5600_I2C_ADDRESS_SLAVE, &reg, 1, false) != ARM_DRIVER_OK) {
        return false; // Error de transmisión
    }

    // Leer 1 byte desde el registro
    if (I2Cdrv->MasterReceive(AS5600_I2C_ADDRESS_SLAVE, &status_sen, 1, false) != ARM_DRIVER_OK) {
        return false; // Error de recepción
    }

    // Verificar bit 5 del registro de estado (MD - Magnet Detected)
    if ((status_sen & (1 << 5)) != 0) {
        return true;  // Imán presente
    } else {
        return false; // Imán no detectado
    }
}
