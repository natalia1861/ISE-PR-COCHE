#ifndef __I2C_PRUEBA_TEMP_H
#define __I2C_PRUEBA_TEMP_H
    
#include "nak_Driver_I2C.h"

#define I2C_temp            I2C1
//I2C register addresses
#define REG_TEMP            0x00
#define m_ADDR              0x48

#define FLAG_TEMP           0x01

void Init_temp_sensor(void);

#endif
