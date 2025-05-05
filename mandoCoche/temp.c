#include "temp.h"

/*--------------------------VARIABLES----------------------------*/

//I2C register addresses
#define REG_TEMP    0x00
#define m_ADDR 0x48

//driver I2C
extern ARM_DRIVER_I2C            Driver_I2C1;
static ARM_DRIVER_I2C *drv_I2C = &Driver_I2C1;

static osMessageQueueId_t mid_MsgQueueTemp;

osThreadId_t tid_Thread_sensor;


/*-------------------PROTOTIPOS DE FUNCIONES---------------------*/
void I2C_SignalEvent_TEMP (uint32_t event);
static void initI2C(void);
static float temp(void);
static int read16(uint8_t reg);
static void Init_MsgQueue_Temp (void);

/*---------------------------FUNCIONES---------------------------*/
//SENSOR TEMPERATURA
/* I2C Signal Event function callback */
void I2C_SignalEvent_TEMP (uint32_t event) {
  if (event & ARM_I2C_EVENT_TRANSFER_DONE) {
    osThreadFlagsSet(tid_Thread_sensor, TRANSFER_TEMP_COMPLETE);
  }
}
static void initI2C(void){
	drv_I2C->Initialize(I2C_SignalEvent_TEMP);
	drv_I2C->PowerControl(ARM_POWER_FULL);
	drv_I2C->Control(ARM_I2C_BUS_SPEED, ARM_I2C_BUS_SPEED_FAST);
	drv_I2C->Control(ARM_I2C_BUS_CLEAR, 0);
}
static float temp(){
    //Signed return value
    short value;
 
    //Read the 11-bit raw temperature value
    value = read16(REG_TEMP) >> 5;
 
    //Sign extend negative numbers
    if (value & (1 << 10))
        value |= 0xFC00;// this operation sets the upper 10 bits of value to 1, while leaving the lower bits unchanged.
 
    //Return the temperature in °C
    return value * 0.125;//So, if the 10th bit of value is set, the upper 10 bits of value will be set to 1 by performing a bitwise OR operation with 0xFC00. If the 10th bit is not set, nothing happens.
}
 
 
static int read16(uint8_t reg){
//Create a temporary buffer
  uint8_t buff[2];

  //Select the register
  drv_I2C->MasterTransmit(m_ADDR, &reg, 1, false);
  /* Wait until transfer completed */
  //osThreadFlagsWait(TRANSFER_TEMP_COMPLETE, osFlagsWaitAny, 10000000);


	//Read the 16-bit register
  //drv_I2C->MasterReceive(m_ADDR, buff, 2, false);
  /* Wait until transfer completed */
  //osThreadFlagsWait(TRANSFER_TEMP_COMPLETE, osFlagsWaitAny, 10000000);
	
	//Return the combined 16-bit value
  return (buff[0] << 8) | buff[1];
}

void leer_temp (void) {
	MSGQUEUE_TEMP_t temperatura;
	temperatura.temp = temp();
	//osMessageQueuePut (mid_MsgQueueTemp, &temperatura, NULL, 5000);
	//osThreadFlagsSet (tid_Thread_sensor, TEMP_FLAG);
}

//COLA
static void Init_MsgQueue_Temp (void) {
  mid_MsgQueueTemp = osMessageQueueNew(SIZE_MSGQUEUE_TEMP , sizeof(MSGQUEUE_TEMP_t), NULL);
}
osMessageQueueId_t getTempQueueID(void){
	return mid_MsgQueueTemp;
}


/*---------------------------MODULO-----------------------------*/
void initModTemp(void){
	initI2C();
	Init_MsgQueue_Temp();
} 

void thread__temp (void *no_argument) {
    float temperature;
    initModTemp();
    while (1) {
        leer_temp();
        osDelay(100);
    }
}
