#ifndef __nak_Driver_I2C_H
#define __nak_Driver_I2C_H

/**
 ******************************************************************************
 * @file           : nak_Driver_I2C.h
 * @author         : Natalia Agüero
 * @date           : 19/10/2024
 * @brief          : Definiciones y funciones del driver I2C
 ******************************************************************************
 * @attention
 * 
 * 
 ******************************************************************************
 */

#include "Driver_I2C.h"
#include "cmsis_os2.h"

#define BUS_I2C_100K_SPEED							ARM_I2C_BUS_SPEED_STANDARD
#define BUS_I2C_400K_SPEED							ARM_I2C_BUS_SPEED_FAST
#define BUS_I2C_1M_SPEED							ARM_I2C_BUS_SPEED_FAST_PLUS
#define BUS_I2C_3_4_M_SPEED							ARM_I2C_BUS_SPEED_HIGH 

//#define AS5600_I2C_ADDRESS					0x36
//#define AS5600_ANGLE_REG    				0x0E

#define FLAG_TRANSFER_COMPLETE					    0x01

typedef enum {
    I2C_LINE_1,
    I2C_LINE_2,
    I2C_LINE_3,
    I2C_LINE_MAX
} I2C_LINE;

//struct for I2C Driver
typedef struct {
	char name[5];
	ARM_DRIVER_I2C *driver;
	osMutexId_t mutex_I2C;
	osSemaphoreId_t transfer_I2C_semaphore;
	ARM_I2C_SignalEvent_t callback_I2C;
	bool initialized;
} I2C_DriverConfig_t;

//hacer algo parecido para gestion de errores

typedef enum {
    I2C_DRIVER_OK,
    I2C_DRIVER_ERROR,
    I2C_DRIVER_ERROR_BUSY,
    I2C_DRIVER_ERROR_TIMEOUT ,
    I2C_DRIVER_ERROR_UNSUPPORTED,
    I2C_DRIVER_ERROR_PARAMETER,
    I2C_DRIVER_ERROR_SPECIFIC
} status_error_codes_t;
 
typedef struct {
    uint32_t speed;
} I2C_configuration_t;

int32_t I2C_Init_All (void);
int32_t I2C_Init(I2C_LINE I2C_line);
int32_t I2C_Configure(I2C_LINE I2C_line, I2C_configuration_t configuration);
int32_t I2C_ReadRegister (I2C_LINE I2C_line, uint32_t SLAVE_ADDRESS, uint8_t reg, uint8_t* data);
int32_t I2C_ReadRegisters (I2C_LINE I2C_line, uint32_t SLAVE_ADDRESS, uint8_t reg, uint8_t* data, uint8_t size);
int32_t I2C_WriteRegister (I2C_LINE I2C_line, uint32_t SLAVE_ADDRESS, uint8_t reg, uint8_t data);
int32_t I2C_TestSensor (I2C_LINE I2C_line, uint32_t slave_address);
#endif
