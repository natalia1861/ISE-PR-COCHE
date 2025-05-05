#ifndef __AS5600_H
#define __AS5600_H

/**
 ******************************************************************************
 * @file           : AS5600.h
 * @author         : Natalia Agüero
 * @date           : 19/10/2024
 * @brief          : Definiciones y funciones del driver AS5600 para la dirección del coche
 ******************************************************************************
 * @attention
 * 
 * Este archivo contiene las definiciones, prototipos y macros para el manejo
 * del sensor AS5600, utilizado para la lectura del ángulo en el control de 
 * dirección del coche.
 * 
 ******************************************************************************
 */

#include "nak_Driver_I2C.h"
#include "cmsis_os2.h"

#define AS5600_I2C_Line 				    I2C_LINE_1

#define AS5600_ADDRESS					    0x36
#define AS5600_WRITE                        0x00
#define AS5600_READ                         0x01
#define AS5600_I2C_ADDR(RW_OP)              (AS5600_ADDRESS << 1) | RW_OP //writes the address and the read/ write operation

#define AS5600_RAW_ANGLE_REG                0x0C
#define AS5600_ANGLE_REG    				0x0E

//REGISTROS
#define ZMCO_REG                            0x00    //ZMCO_REG: how many times ZPOS and MPOS have been permanently written (max: 3)
#define ZPOS_REG                            0x01    //ZPOS_REG: indicates the start position (0º)
#define MPOS_REG                            0x03    //MPOS_REG: indicates the stop position
#define MANG_REG                            0x05    //MANG_REG: indicates the size of the angular range
#define CONF_REG                            0x07    //CONF_REG: explained under
#define RAW_ANGLE_REG                       0x0C
#define ANGLE_REG                           0x0E
#define STATUS_REG                          0x0B    //STATUS_REG: Indicates the current state of sensor. MH magnet too strong, ML magnet too low, MD magnet was detected
#define AGC_REG                             0x1A    //AGC_REG: indicates the gane of the automatic gain control to compensate variations of magnetic field strenght due to changes of temperature, airgap, etc (0-128 in 3.3V)
#define MAGNITUDE_REG                       0x1B    //MAGNITUDE_REG: indicates the magnitude value of the internal CORDIC.
#define BURN_REG                            0xFF

//range can be set by programming a (ZPOS and MPOS) or the MANG

//MASK FOR STATUS REG
typedef enum
{
     SM_Magnet_not_present = 0x00,   // 
     SM_Magnet_high = 0x08,   // Bit 3 (Magnet too High)
     SM_Magnet_low  = 0x10,   // Bit 4 (Magnet too Low)
     SM_Magnet_Detected  = 0x20,   // Bit 5 (Magnet Detected)
} AS5600_status_magnet_t;


//typedef enum {
//    AS5600_OK,
//    AS5600_ERROR,
//    AS5600_TIMEOUT,
//    AS5600_BUSY
//} AS5600_Status;

//CONFG MASKS
#define PM_MASK     0x03    // 00 = NOM, 01 = LPM1, 10 = LPM2, 11 = LPM3
#define HYST_MASK   0x0C    // 00 = OFF, 01 = 1 LSB, 10 = 2 LSBs, 11 = 3 LSBs
#define OUTS_MASK   0x30    // 00 = analog (full range), 01 = analog (reduced range), 10 = digital PWM
#define PWMF_MASK   0xC0    // 00 = 115 Hz, 01 = 230 Hz, 10 = 460 Hz, 11 = 920 Hz
#define SF_MASK     0x300   // 00 = 16x, 01 = 8x, 10 = 4x, 11 = 2x
#define FTH_MASK    0x1C00  // 000 = slow filter only, 001 = 6 LSBs, 010 = 7 LSBs, etc.
#define WD_MASK     0x2000  // 0 = OFF, 1 = ON

//CONF REGISTER POSIBILITIES

//low power mode: 
typedef enum {
    PM_NOM  = 0,     //  6.5 mA
    PM_LPM1 = 1,     //  3.4 mA
    PM_LPM2 = 2,     //  1.8 mA
    PM_LPM3 = 3      //  1.5 mA
} POWER_Mode;

//Hysteheresis: supresses toggling the OUT pin when the magnet is close to zero or 360 degrees.
typedef enum {
    HYS_OFF  = 0,
    HYS_LSB1 = 1,
    HYS_LSB2 = 2,
    HYS_LSB3 = 3   
} Hysteresis_Mode;

//Output stage: choose between analog radiometric output (default) and digital PWM output. If PWM is selected, DAC is powered down.
//In both cases, an external unit can read the angle from ANGLE REGISTER through I2C.
typedef enum {
    OUTS_ANALOG_FULL    = 0,      //DEFAULT
    OUTS_ANALOG_REDUCED = 1,
    OUTS_DIGITAL_PWM    = 2
} Output_State_Mode;

typedef enum {
    PWM_115_Hz = 0,
    PWM_230_Hz,
    PWM_460_Hz,
    PWM_920_Hz
} PWM_Frequency_Mode;

//Slow filter: digital post-processing. If fast filter is off, the slow linear filter is active. 
typedef enum {
    SF_x16 = 0,    //forced in Low Power Mode (LPM)    //2.2 ms delay, 0.015 RMS noise
    SF_x8,                                             //1.1 ms delay, 0.021 RMS noise
    SF_x4,                                             //0.55 ms delay, 0.030 RMS noise
    SF_x2                                              //0.286 ms delay, 0.043 RMS noise
} Slow_Filter_Mode;                                             

//fast filter. It only works if the input variation is greater than the fast filter threshold.
typedef enum {
    FFTH_slow   = 0,           //slow filter only
    FFTH_6_LSB  = 1,
    FFTH_7_LSB  = 2,
    FFTH_9_LSB  = 3,
    FFTH_18_LSB = 4,
    FFTH_21_LSB = 5,
    FFTH_24_LSB = 6,
    FFTH_10_LSB = 7
} Fast_Filter_Threshold_Mode;

//Watchdog: allows saving power by switching into LMP3 if the angle stays within the watchdog threshold of 4LSB for at least one minute
typedef enum {
    WD_OFF = 0,       
    WD_ON
} Watchdog_Mode;

typedef struct {
    POWER_Mode power_mode;
    Hysteresis_Mode hysteresis_mode;
    Output_State_Mode output_mode;
    PWM_Frequency_Mode pwm_mode;
    Slow_Filter_Mode slow_filter_mode;
    Fast_Filter_Threshold_Mode fast_filter_mode;
    Watchdog_Mode watchdog_mode;
} AS5600_Configuration_t;

typedef enum {
    AS5600_OK = 0,              // 0
    AS5600_I2C_Error,           // 1
    AS5600_Test_Error,          // 2
    AS5600_Conf_Error,          // 3
    AS5600_Read_Error,          // 4
    AS5600_Magnet_Too_Strong,   // 5
    AS5600_Magnet_Not_Present,  // 6
    AS5600_Magnet_Too_Weak      //7
} AS5600_Errors;


AS5600_Errors AS5600_Start (I2C_LINE I2C_line);
AS5600_Errors AS5600_Init (I2C_LINE I2C_line);
AS5600_Errors AS5600_Configure (I2C_LINE I2C_line, AS5600_Configuration_t configuracion);
AS5600_Errors AS5600_ReadAngle(I2C_LINE I2C_line, float* angle_degrees);
AS5600_Configuration_t getDefaultConfiguration (void);
AS5600_Errors isMagnetPresent (I2C_LINE I2C_line);

#endif
