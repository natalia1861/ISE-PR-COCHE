/*------------------------------------------------------------------------------
 * MDK Middleware - Component ::Network
 * Copyright (c) 2004-2019 Arm Limited (or its affiliates). All rights reserved.
 *------------------------------------------------------------------------------
 * Name:    HTTP_Server.c
 * Purpose: HTTP Server example
 *----------------------------------------------------------------------------*/

#include <stdio.h>

#include "main.h"

#include "rl_net.h"                     // Keil.MDK-Pro::Network:CORE

#include "stm32f4xx_hal.h"              // Keil::Device:STM32Cube HAL:Common
#include "nak_led.h"                  // ::Board Support:LED
#include "pot.h"
#include "lcd.h"
#include "RTC.h"
#include "gpio.h"
#include "nRF24L01_TX.h"

#define FLAG_SERVER 0x01
#define FLAG_RTC 0x02

//#include "Board_Buttons.h"              // ::Board Support:Buttons
//#include "Board_ADC.h"                  // ::Board Support:A/D Converter
//#include "Board_GLCD.h"                 // ::Board Support:Graphic LCD
//#include "GLCD_Config.h"                // Keil.MCBSTM32F400::Board Support:Graphic LCD

// Main stack size must be multiple of 8 Bytes
#define APP_MAIN_STK_SZ (1024U)
uint64_t app_main_stk[APP_MAIN_STK_SZ / 8];
const osThreadAttr_t app_main_attr = {
  .stack_mem  = &app_main_stk[0],
  .stack_size = sizeof(app_main_stk)
};

//extern GLCD_FONT GLCD_Font_6x8;
//extern GLCD_FONT GLCD_Font_16x24;

extern uint16_t AD_in          (uint32_t ch);
extern uint8_t  get_button     (void);
extern void     netDHCP_Notify (uint32_t if_num, uint8_t option, const uint8_t *val, uint32_t len);

extern bool LEDrun;
extern bool lcd_stop;
extern char lcd_text[2][20+1];

extern osThreadId_t TID_Display;
extern osThreadId_t TID_Led;

bool LEDrun;
bool LED_Rrun;
bool LED_Grun;
char lcd_text[2][20+1] = { "LCD line 1",
                           "LCD line 2" };

/* Thread IDs */
osThreadId_t TID_Display;
osThreadId_t TID_Led;
osThreadId_t TID_RTC;

/* Thread declarations */
//static void BlinkLed (void *arg);
//static void Display  (void *arg);

__NO_RETURN void app_main (void *arg);

/* Read analog inputs */
uint16_t AD_in (uint32_t ch) {
  int32_t val = 0;

  if (ch == 0) {
    ADC_StartConversion();
    while (ADC_ConversionDone () < 0);
    val = ADC_GetValue();
  }
  return ((uint16_t)val);
}

/* Read digital inputs */
//uint8_t get_button (void) {
//  //return ((uint8_t)Buttons_GetState ());
//}

/* IP address change notification */
void netDHCP_Notify (uint32_t if_num, uint8_t option, const uint8_t *val, uint32_t len) {

  (void)if_num;
  (void)val;
  (void)len;

  if (option == NET_DHCP_OPTION_IP_ADDRESS) {
    /* IP address change, trigger LCD update */
    osThreadFlagsSet (TID_Display, 0x01);
  }
}

/*----------------------------------------------------------------------------
  Thread 'Display': LCD display handler
 *---------------------------------------------------------------------------*/
//static __NO_RETURN void Display (void *arg) {
//  //static uint8_t ip_addr[NET_ADDR_IP6_LEN];
//  static char    ip_ascii[40];
//  static char    buf[24];
//  //uint32_t x = 0;
//  static uint32_t flags;

//  (void)arg;

//  LCD_start();
//  LCD_clean();
//  LCD_write(1, "IP4:Waiting for DHCP");
//  sprintf (buf, "IP6:%.16s", ip_ascii);
//  LCD_write(2, buf);
//  sprintf (buf, "%s", ip_ascii+16);
//  
//  while(1) {
//    /* Wait for signal from DHCP */
//     flags = osThreadFlagsWait (FLAG_SERVER | FLAG_RTC, osFlagsWaitAny, osWaitForever);
//     /* Display user text lines */
//		switch (flags) {
//			case FLAG_SERVER:
//			 sprintf (buf, "%-20s", lcd_text[0]);
//			 LCD_write(1, buf);
//			 sprintf (buf, "%-20s", lcd_text[1]);
//			 LCD_write(2, buf);
//			break;
//			case FLAG_RTC:
////			 sprintf (buf, "%-20s", rtc_date_time[0]);
////			 LCD_write(1, buf);
////			 sprintf (buf, "%-20s", rtc_date_time[1]);
////			 LCD_write(2, buf);
//			break;
//			
//		}
//  }
//}

/*----------------------------------------------------------------------------
  Thread 'BlinkLed': Blink the LEDs on an eval board
 *---------------------------------------------------------------------------*/
//static __NO_RETURN void BlinkLed (void *arg) {
//  const uint8_t led_val[16] = { 0x48,0x88,0x84,0x44,0x42,0x22,0x21,0x11,
//                                0x12,0x0A,0x0C,0x14,0x18,0x28,0x30,0x50 };
//	uint8_t led_red = 0x04;
//	uint8_t green_red = 0x01;											
//  uint32_t cnt = 0U;

//  (void)arg;

//  LEDrun = true;
//  while(1) {
//    /* Every 100 ms */
//		if (LED_Rrun == true) {
//				LED_SetOut (led_red);
//				led_red ^= 0x04;
//		} else if (LED_Grun == true) {
//				LED_SetOut (green_red);
//				green_red ^= 0x01;
//		} else if (LEDrun == true) {
//      LED_SetOut (led_val[cnt]);
//      if (++cnt >= sizeof(led_val)) {
//        cnt = 0U;
//      }
//    }
//    osDelay (100);
//  }
//}

/*----------------------------------------------------------------------------
  Main Thread 'main': Run Network
 *---------------------------------------------------------------------------*/
__NO_RETURN void app_main (void *arg) {
  (void)arg;

 //LED_Initialize();
 //ADC_Initialize();
 // Buttons_Initialize();
  
 //init_RTC();
  //RTC_getTime_Date();
  //init_pulsador();
  netInitialize ();

  //TID_Led     = osThreadNew (BlinkLed, NULL, NULL);
  //TID_Display = osThreadNew (Display,  NULL, NULL);
  //TID_RTC 		= osThreadNew (Rtc_func, NULL, NULL);
  //Init_RF_TX();

  osThreadExit();
}
