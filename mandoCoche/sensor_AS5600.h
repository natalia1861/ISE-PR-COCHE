#ifndef __SENSOR_AS5600_H
#define __SENSOR_AS5600_H

#include "Driver_I2C.h"
#include "stm32f4xx_hal.h"

int Init_AS5600_Thread(void);

typedef struct{
	uint8_t reg_addr;
  uint8_t content_rx[8];
}in_buffer_t;

//typedef enum
//{
//     MH_MASK  = 0x08,   // Bit 3 (Magnet too High)
//     ML_MASK  = 0x10,   // Bit 4 (Magnet too Low)
//     MD_MASK  = 0x20,   // Bit 5 (Magnet Detected)
//} AS5600_status_magnet_t;

//typedef enum {
//    AS5600_OK = 0,              // 0
//    AS5600_I2C_Error,           // 1
//    AS5600_Test_Error,          // 2
//    AS5600_Conf_Error,          // 3
//    AS5600_Read_Error,          // 4
//    AS5600_Magnet_Too_Strong,   // 5
//    AS5600_Magnet_Not_Present   // 6
//} AS5600_Errors;
  
//SENSOR++++++++++++++++++++++++++++
//low power mode: 
//typedef enum {
//    PM_NOM  = 0,     //  6.5 mA
//    PM_LPM1 = 1,     //  3.4 mA
//    PM_LPM2 = 2,     //  1.8 mA
//    PM_LPM3 = 3      //  1.5 mA
//} POWER_Mode;

////Hysteheresis: supresses toggling the OUT pin when the magnet is close to zero or 360 degrees.
//typedef enum {
//    HYS_OFF  = 0,
//    HYS_LSB1 = 1,
//    HYS_LSB2 = 2,
//    HYS_LSB3 = 3   
//} Hysteresis_Mode;

////Output stage: choose between analog radiometric output (default) and digital PWM output. If PWM is selected, DAC is powered down.
////In both cases, an external unit can read the angle from ANGLE REGISTER through I2C.
//typedef enum {
//    OUTS_ANALOG_FULL    = 0,      //DEFAULT
//    OUTS_ANALOG_REDUCED = 1,
//    OUTS_DIGITAL_PWM    = 2
//} Output_State_Mode;

//typedef enum {
//    PWM_115_Hz = 0,
//    PWM_230_Hz,
//    PWM_460_Hz,
//    PWM_920_Hz
//} PWM_Frequency_Mode;

////Slow filter: digital post-processing. If fast filter is off, the slow linear filter is active. 
//typedef enum {
//    SF_x16 = 0,    //forced in Low Power Mode (LPM)    //2.2 ms delay, 0.015 RMS noise
//    SF_x8,                                             //1.1 ms delay, 0.021 RMS noise
//    SF_x4,                                             //0.55 ms delay, 0.030 RMS noise
//    SF_x2                                              //0.286 ms delay, 0.043 RMS noise
//} Slow_Filter_Mode;                                             

////fast filter. It only works if the input variation is greater than the fast filter threshold.
//typedef enum {
//    FFTH_slow   = 0,           //slow filter only
//    FFTH_6_LSB  = 1,
//    FFTH_7_LSB  = 2,
//    FFTH_9_LSB  = 3,
//    FFTH_18_LSB = 4,
//    FFTH_21_LSB = 5,
//    FFTH_24_LSB = 6,
//    FFTH_10_LSB = 7
//} Fast_Filter_Threshold_Mode;

////Watchdog: allows saving power by switching into LMP3 if the angle stays within the watchdog threshold of 4LSB for at least one minute
//typedef enum {
//    WD_OFF = 0,       
//    WD_ON
//} Watchdog_Mode;

//typedef struct {
//    POWER_Mode power_mode;
//    Hysteresis_Mode hysteresis_mode;
//    Output_State_Mode output_mode;
//    PWM_Frequency_Mode pwm_mode;
//    Slow_Filter_Mode slow_filter_mode;
//    Fast_Filter_Threshold_Mode fast_filter_mode;
//    Watchdog_Mode watchdog_mode;
//} AS5600_Configuration_t;
#endif //__SENSOR_AS5600_H



/*****************************************************************************/

