#ifndef __SENSOR_AS5600_H
#define __SENSOR_AS5600_H

#include "Driver_I2C.h"
#include "stm32f4xx_hal.h"

typedef enum
{
    AS5600_OK,
    AS5600_ERROR
} AS5600_status_t;

typedef struct{
	uint8_t reg_addr;
  uint8_t content_rx[8];
}in_buffer_t;

typedef enum
{
     MH_MASK  = 0x08,   // Bit 3 (Magnet too High)
     ML_MASK  = 0x10,   // Bit 4 (Magnet too Low)
     MD_MASK  = 0x20,   // Bit 5 (Magnet Detected)
} AS5600_status_magnet_t;

int as5600_init(void);
AS5600_status_t as5600_readout(float* read_angle);
#endif 



/*****************************************************************************/

