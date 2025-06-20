#ifndef __SENSOR_AS5600_H
#define __SENSOR_AS5600_H

#include "Driver_I2C.h"
#include "stm32f4xx_hal.h"

typedef enum
{
    AS5600_OK,
    AS5600_MAGNET_NOT_DETECTED,
    AS5600_ERROR
} AS5600_status_t;

typedef struct{
	uint8_t reg_addr;
  uint8_t content_rx[8];
}in_buffer_t;

typedef enum
{
     MH_BIT  = 3,   // Bit 3 (Magnet too High)
     ML_BIT  = 4,   // Bit 4 (Magnet too Low)
     MD_BIT  = 5,   // Bit 5 (Magnet Detected)
} AS5600_status_magnet_t;

#define GET_BIT_MASK(bit)       (1 << bit)

AS5600_status_t as5600_init(void);
AS5600_status_t as5600_readout(float* read_angle);
#endif 



/*****************************************************************************/

