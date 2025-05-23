#ifndef __MODO_SLEEP_H
#define __MODO_SLEEP_H

#include "Driver_SPI.h"
#include "stm32f4xx_hal.h"
#include "stm32f4xx_hal_eth.h"
#include <string.h>
#include "stm32f4xx_hal_gpio.h"
#include "stdio.h"

#define PHY_PWR_DOWN       (1<<11)
#define PHY_ADDRESS        0x00 /* default ADDR for PHY: LAN8742 */
#define RTC_ASYNCH_PREDIV    0x7F
#define RTC_SYNCH_PREDIV     0x0130
#define LAN8742A_PHY_ADDRESS            0x00U
#define SLEEP_MODE 0x01U

#define HAL_ETH_LEGACY_MODULE_ENABLED
#define HAL_FLASH_MODULE_ENABLED

void ETH_PhyExitFromPowerDownMode(void);
void ETH_PhyEnterPowerDownMode(void);
void SleepMode_Measure(void);

HAL_StatusTypeDef HAL_ETH_WritePHYRegister(ETH_HandleTypeDef *heth, uint16_t PHYReg, uint32_t RegValue);
HAL_StatusTypeDef HAL_ETH_ReadPHYRegister(ETH_HandleTypeDef *heth, uint16_t PHYReg, uint32_t *RegValue);

#endif /* __MODO_SLEEP_H */
