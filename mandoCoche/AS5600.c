/**
 ******************************************************************************
 * @file           : AS5600.c
 * @author         : Natalia Agüero
 * @date           : 19/10/2024
 * @brief          : AS5600 sensor
 ******************************************************************************
 * @attention
 * 
 * Este archivo contiene la implementación del driver para el sensor AS5600,
 * utilizado para la medición del ángulo en el sistema de dirección del coche.
 * 
 ******************************************************************************
 */
 
#include "AS5600.h"

uint32_t debug;

AS5600_Errors AS5600_Start (I2C_LINE I2C_line) {
    AS5600_Configuration_t conf = getDefaultConfiguration();
    
    if (I2C_Init_All() != osOK)
    {
        return AS5600_I2C_Error;
    }
    if (AS5600_Init(I2C_line) != AS5600_OK)
    {
        return AS5600_I2C_Error;
    }
    
    if (AS5600_Configure(I2C_line, conf)!= AS5600_OK)
    {
        return AS5600_Conf_Error;
    }
    return AS5600_OK;
//    if (isMagnetPresent(I2C_line) != AS5600_OK)
//    {
//        return AS5600_Test_Error;
//    }
}
AS5600_Errors AS5600_Init (I2C_LINE I2C_line) {
    if (I2C_Init(I2C_line) != I2C_DRIVER_OK) {
        return AS5600_I2C_Error;
    }
    
//    if (I2C_TestSensor (I2C_line, AS5600_ADDRESS) != I2C_DRIVER_OK) {
//        return AS5600_Test_Error;
//    }
    
    return AS5600_OK;
}

AS5600_Errors AS5600_Configure (I2C_LINE I2C_line, AS5600_Configuration_t configuracion) {
    uint8_t conf_buffer[2];
    uint8_t read_config[2];
    uint16_t configuration_value;
    if (I2C_ReadRegisters(I2C_line, AS5600_ADDRESS, CONF_REG, &read_config[0], 2) != AS5600_OK)
    {
        return AS5600_Read_Error; // Error en la lectura, retorna falso
    }
    debug = (uint16_t)((configuracion.fast_filter_mode  << 10)& FTH_MASK);

    // Configuración del registro (13 bits)
    configuration_value = (uint16_t)((configuracion.power_mode   << 0) & PM_MASK )  |
                     (uint16_t)((configuracion.hysteresis_mode   << 2) & HYST_MASK) |
                     (uint16_t)((configuracion.output_mode       << 4) & OUTS_MASK) |
                     (uint16_t)((configuracion.pwm_mode          << 6) & PWMF_MASK) |
                     (uint16_t)((configuracion.slow_filter_mode  << 8) & SF_MASK)   |
                     (uint16_t)((configuracion.fast_filter_mode  << 10)& FTH_MASK)  |
                     (uint16_t)((configuracion.watchdog_mode     << 11)& WD_MASK);

    conf_buffer[0] = (uint8_t)(configuration_value & 0xFF);      // Byte bajo
    conf_buffer[1] = (uint8_t)((configuration_value >> 8) & 0xFF); // Byte alto
    
    if (conf_buffer [0] != read_config [0])
    {
        if (I2C_WriteRegister(I2C_line, AS5600_ADDRESS, CONF_REG, conf_buffer[0]) != ARM_DRIVER_OK)
        {
            // Error en la transmisión del primer byte
            return AS5600_Conf_Error;
        }
    }  

    if (conf_buffer [1] != read_config [1])
    {
        if (I2C_WriteRegister(I2C_line, AS5600_ADDRESS, CONF_REG+1, conf_buffer[1]) != ARM_DRIVER_OK)
        {
            // Error en la transmisión del segundo byte
            return AS5600_Conf_Error;
        }
    }
    
    osDelay(10);
    
    if (I2C_ReadRegisters(I2C_line, AS5600_ADDRESS, CONF_REG, &read_config[0], 2) != AS5600_OK)
    {
        return AS5600_Read_Error; // Error en la lectura, retorna falso
    }
    
    if (read_config[0] != conf_buffer[0] || read_config [1] != conf_buffer[1])
    {
        return AS5600_Conf_Error;
    }
    return AS5600_OK;
}

AS5600_Errors AS5600_ReadAngle(I2C_LINE I2C_line, float* angle_degrees) {
    uint8_t angle_data[2]; // Array para almacenar los 2 bytes de los registros de ángulo
    uint16_t angle = 0;

    // Leer los registros 0x0E (bajo) y 0x0F (alto) en una sola llamada a I2C_ReadRegisters
    if (I2C_ReadRegisters(I2C_line, AS5600_ADDRESS, AS5600_ANGLE_REG, angle_data, 2) != ARM_DRIVER_OK) {
        return AS5600_Read_Error; // Error en la lectura del ángulo
    }

    // Combinar los dos bytes de los registros 0x0E (bajo) y 0x0F (alto) en un valor de 16 bits
    angle = ((uint16_t)angle_data[1] << 8) | (uint16_t)angle_data[0];

    // Convertir el valor a grados y guardarlo en el puntero proporcionado
    *angle_degrees = ((float)angle / 4096.0f) * 360.0f;

    return AS5600_OK; // Retornar el estado
}


AS5600_Configuration_t getDefaultConfiguration (void) {
    AS5600_Configuration_t conf;

    // Establecer los valores predeterminados para cada campo de la configuración
    conf.power_mode = PM_NOM;               // Modo de potencia NOM (Normal)
    conf.hysteresis_mode = HYS_OFF;         // Histeresis desactivada (OFF)
    conf.output_mode = OUTS_ANALOG_FULL;    // Salida analógica (rango completo)
    conf.pwm_mode = PWM_115_Hz;             // Frecuencia PWM a 115 Hz
    conf.slow_filter_mode = SF_x8;         // Filtro lento x16 
    conf.fast_filter_mode = FFTH_6_LSB;     // Umbral de filtro rápido 6 LSBs
    conf.watchdog_mode = WD_OFF;            // Watchdog desactivado (OFF)

    return conf;
}

AS5600_Errors isMagnetPresent (I2C_LINE I2C_line) {
    uint8_t config_reg[2];
    uint8_t status_reg;
        // Leer el registro de estado debug
    if (I2C_ReadRegisters(I2C_line, AS5600_ADDRESS, CONF_REG, &config_reg[0], 2) != AS5600_OK)
    {
        return AS5600_Read_Error; // Error en la lectura, retorna falso
    }
    
    if (I2C_ReadRegisters(I2C_line, AS5600_ADDRESS, STATUS_REG, &status_reg, 1) != AS5600_OK)
    {
        return AS5600_Read_Error; // Error en la lectura, retorna falso
    }
    
    // Verificar el bit MD para ver si el imán está detectado
    switch ((AS5600_status_magnet_t) status_reg)
    {
        case SM_Magnet_not_present:
            return AS5600_Magnet_Not_Present;
        case SM_Magnet_Detected: // El imán está presente
            return AS5600_OK;
        case SM_Magnet_high:
            return AS5600_Magnet_Too_Strong;
        case SM_Magnet_low:
            return AS5600_Magnet_Too_Weak; // El imán no está presente
        default:
            return AS5600_Read_Error;
    }
}
