/*------------------------------------------------------------------------------
 * MDK Middleware - Component ::Network:Service
 * Copyright (c) 2004-2018 ARM Germany GmbH. All rights reserved.
 *------------------------------------------------------------------------------
 * Name:    HTTP_Server_CGI.c
 * Purpose: HTTP Server CGI Module
 * Rev.:    V6.0.0
 *----------------------------------------------------------------------------*/

#include <stdio.h>
#include <string.h>
#include "cmsis_os2.h"                  // ::CMSIS:RTOS2
#include "rl_net.h"                     // Keil.MDK-Pro::Network:CORE

#include "nak_led.h"                  
#include "rtc.h"
#include "flash.h"
#include "app_main.h"

#define FLAG_SERVER 0x01

#if      defined (__ARMCC_VERSION) && (__ARMCC_VERSION >= 6010050)
#pragma  clang diagnostic push
#pragma  clang diagnostic ignored "-Wformat-nonliteral"
#endif

extern char hora[80];
extern char fecha[80];
extern char marcha_S[80];
extern char distancia_S[80];
extern char direccion_S[80];
extern char consumo_S [80];

//from app_main.c
extern float medidas_consumo[NUM_MAX_MUESTRA_CONSUMO];
extern char horas_consumo[NUM_MAX_MUESTRA_CONSUMO][FLASH_NUM_CHAR_HORA];

//local variables for print in web -> seria mejor dejar estas como globales y que se copien, no que tenga acceso a las de arriba
char consumo_flash_S[80];
char hora_flash_S[9];

// http_server.c
//extern uint16_t AD_in (uint32_t ch);
extern uint8_t  get_button (void);

extern bool LEDrun;
extern char lcd_text[2][LCD_MAX_CHARACTERS+1];
extern char rtc_date_time[RTC_MAX][LCD_MAX_CHARACTERS+1];
extern osThreadId_t TID_Display;

// Local variables.
static uint8_t P2;
static uint8_t ip_addr[NET_ADDR_IP6_LEN];
static char    ip_string[40];

bool lcd_stop = false;

// My structure of CGI status variable.
typedef struct {
  uint8_t idx;
  uint8_t unused[3];
} MY_BUF;
#define MYBUF(p)        ((MY_BUF *)p)

// Process query string received by GET request.
void netCGI_ProcessQuery (const char *qstr) {
  netIF_Option opt = netIF_OptionMAC_Address;
  int16_t      typ = 0;
  char var[40];

  do {
    // Loop through all the parameters
    qstr = netCGI_GetEnvVar (qstr, var, sizeof (var));
    // Check return string, 'qstr' now points to the next parameter

    switch (var[0]) {
      case 'i': // Local IP address
        if (var[1] == '4') { opt = netIF_OptionIP4_Address;       }
        else               { opt = netIF_OptionIP6_StaticAddress; }
        break;

      case 'm': // Local network mask
        if (var[1] == '4') { opt = netIF_OptionIP4_SubnetMask; }
        break;

      case 'g': // Default gateway IP address
        if (var[1] == '4') { opt = netIF_OptionIP6_DefaultGateway; }
        else               { opt = netIF_OptionIP6_DefaultGateway; }
        break;

      case 'p': // Primary DNS server IP address
        if (var[1] == '4') { opt = netIF_OptionIP4_PrimaryDNS; }
        else               { opt = netIF_OptionIP6_PrimaryDNS; }
        break;

      case 's': // Secondary DNS server IP address
        if (var[1] == '4') { opt = netIF_OptionIP4_SecondaryDNS; }
        else               { opt = netIF_OptionIP6_SecondaryDNS; }
        break;
      
      default: var[0] = '\0'; break;
    }

    switch (var[1]) {
      case '4': typ = NET_ADDR_IP4; break;
      case '6': typ = NET_ADDR_IP6; break;

      default: var[0] = '\0'; break;
    }

    if ((var[0] != '\0') && (var[2] == '=')) {
      netIP_aton (&var[3], typ, ip_addr);
      // Set required option
      netIF_SetOption (NET_IF_CLASS_ETH, opt, ip_addr, sizeof(ip_addr));
    }
  } while (qstr);
}

// Process data received by POST request.
// Type code: - 0 = www-url-encoded form data.
//            - 1 = filename for file upload (null-terminated string).
//            - 2 = file upload raw data.
//            - 3 = end of file upload (file close requested).
//            - 4 = any XML encoded POST data (single or last stream).
//            - 5 = the same as 4, but with more XML data to follow.
void netCGI_ProcessData (uint8_t code, const char *data, uint32_t len) {
  char var[40],passw[12];

  if (code != 0) {
    // Ignore all other codes
    return;
  }

  P2 = 0;
  LEDrun = true;
  if (len == 0) {
    // No data or all items (radio, checkbox) are off
    //LED_SetOut (P2);
    return;
  }
  passw[0] = 1;
  do {
    // Parse all parameters
    data = netCGI_GetEnvVar (data, var, sizeof (var));
    if (var[0] != 0) {
      // First character is non-null, string exists
      if (strcmp (var, "led0=on") == 0) {
        P2 |= 0x01;
      }
      else if (strcmp (var, "led1=on") == 0) {
        P2 |= 0x02;
      }
      else if (strcmp (var, "led2=on") == 0) {
        P2 |= 0x04;
      }
      else if (strcmp (var, "led3=on") == 0) {
        P2 |= 0x08;
      }
      else if (strcmp (var, "led4=on") == 0) {
        P2 |= 0x10;
      }
      else if (strcmp (var, "led5=on") == 0) {
        P2 |= 0x20;
      }
      else if (strcmp (var, "led6=on") == 0) {
        P2 |= 0x40;
      }
      else if (strcmp (var, "led7=on") == 0) {
        P2 |= 0x80;
      }
      else if (strcmp (var, "ctrl=Browser") == 0) {
        LEDrun = false;
      }
      else if ((strncmp (var, "pw0=", 4) == 0) ||
               (strncmp (var, "pw2=", 4) == 0)) {
        // Change password, retyped password
        if (netHTTPs_LoginActive()) {
          if (passw[0] == 1) {
            strcpy (passw, var+4);
          }
          else if (strcmp (passw, var+4) == 0) {
            // Both strings are equal, change the password
            netHTTPs_SetPassword (passw);
          }
        }
      }
      else if (strncmp (var, "lcd1=", 5) == 0) {
        // LCD Module line 1 text
        strcpy (lcd_text[0], var+5);
        osThreadFlagsSet (TID_Display, FLAG_SERVER);
      }
      else if (strncmp (var, "lcd2=", 5) == 0) {
        // LCD Module line 2 text
        strcpy (lcd_text[1], var+5);
        osThreadFlagsSet (TID_Display, FLAG_SERVER);
      }
    }
  } while (data);
  //LED_SetOut (P2);
}

// Generate dynamic web data from a script line.
uint32_t netCGI_Script (const char *env, char *buf, uint32_t buflen, uint32_t *pcgi) {
  int32_t socket;
  netTCP_State state;
  NET_ADDR r_client;
  const char *lang;
  uint32_t len = 0U;
  uint8_t id;
  static uint32_t adv;
  netIF_Option opt = netIF_OptionMAC_Address;
  int16_t      typ = 0;

  switch (env[0]) {
    // Analyze a 'c' script line starting position 2
    case 'a' :
      // Network parameters from 'network.cgi'
      switch (env[3]) {
        case '4': typ = NET_ADDR_IP4; break;
        case '6': typ = NET_ADDR_IP6; break;

        default: return (0);
      }
      
      switch (env[2]) {
        case 'l':
          // Link-local address
          if (env[3] == '4') { return (0);                             }
          else               { opt = netIF_OptionIP6_LinkLocalAddress; }
          break;

        case 'i':
          // Write local IP address (IPv4 or IPv6)
          if (env[3] == '4') { opt = netIF_OptionIP4_Address;       }
          else               { opt = netIF_OptionIP6_StaticAddress; }
          break;

        case 'm':
          // Write local network mask
          if (env[3] == '4') { opt = netIF_OptionIP4_SubnetMask; }
          else               { return (0);                       }
          break;

        case 'g':
          // Write default gateway IP address
          if (env[3] == '4') { opt = netIF_OptionIP4_DefaultGateway; }
          else               { opt = netIF_OptionIP6_DefaultGateway; }
          break;

        case 'p':
          // Write primary DNS server IP address
          if (env[3] == '4') { opt = netIF_OptionIP4_PrimaryDNS; }
          else               { opt = netIF_OptionIP6_PrimaryDNS; }
          break;

        case 's':
          // Write secondary DNS server IP address
          if (env[3] == '4') { opt = netIF_OptionIP4_SecondaryDNS; }
          else               { opt = netIF_OptionIP6_SecondaryDNS; }
          break;
      }

      netIF_GetOption (NET_IF_CLASS_ETH, opt, ip_addr, sizeof(ip_addr));
      netIP_ntoa (typ, ip_addr, ip_string, sizeof(ip_string));
      len = (uint32_t)sprintf (buf, &env[5], ip_string);
      break;

    case 'b':
      // LED control from 'led.cgi'
      if (env[2] == 'c') {
        // Select Control
        len = (uint32_t)sprintf (buf, &env[4], LEDrun ?     ""     : "selected",
                                               LEDrun ? "selected" :    ""     );
        break;
      }
      // LED CheckBoxes
      id = env[2] - '0';
      if (id > 7) {
        id = 0;
      }
      id = (uint8_t)(1U << id);
      len = (uint32_t)sprintf (buf, &env[4], (P2 & id) ? "checked" : "");
      break;

    case 'c':
      // TCP status from 'tcp.cgi'
      while ((uint32_t)(len + 150) < buflen) {
        socket = ++MYBUF(pcgi)->idx;
        state  = netTCP_GetState (socket);

        if (state == netTCP_StateINVALID) {
          /* Invalid socket, we are done */
          return ((uint32_t)len);
        }

        // 'sprintf' format string is defined here
        len += (uint32_t)sprintf (buf+len,   "<tr align=\"center\">");
        if (state <= netTCP_StateCLOSED) {
          len += (uint32_t)sprintf (buf+len, "<td>%d</td><td>%d</td><td>-</td><td>-</td>"
                                             "<td>-</td><td>-</td></tr>\r\n",
                                             socket,
                                             netTCP_StateCLOSED);
        }
        else if (state == netTCP_StateLISTEN) {
          len += (uint32_t)sprintf (buf+len, "<td>%d</td><td>%d</td><td>%d</td><td>-</td>"
                                             "<td>-</td><td>-</td></tr>\r\n",
                                             socket,
                                             netTCP_StateLISTEN,
                                             netTCP_GetLocalPort(socket));
        }
        else {
          netTCP_GetPeer (socket, &r_client, sizeof(r_client));

          netIP_ntoa (r_client.addr_type, r_client.addr, ip_string, sizeof (ip_string));
          
          len += (uint32_t)sprintf (buf+len, "<td>%d</td><td>%d</td><td>%d</td>"
                                             "<td>%d</td><td>%s</td><td>%d</td></tr>\r\n",
                                             socket, netTCP_StateLISTEN, netTCP_GetLocalPort(socket),
                                             netTCP_GetTimer(socket), ip_string, r_client.port);
        }
      }
      /* More sockets to go, set a repeat flag */
      len |= (1u << 31);
      break;

    case 'd':
      // System password from 'system.cgi'
      switch (env[2]) {
        case '1':
          len = (uint32_t)sprintf (buf, &env[4], netHTTPs_LoginActive() ? "Enabled" : "Disabled");
          break;
        case '2':
          len = (uint32_t)sprintf (buf, &env[4], netHTTPs_GetPassword());
          break;
      }
      break;

    case 'e':
      // Browser Language from 'language.cgi'
      lang = netHTTPs_GetLanguage();
      if      (strncmp (lang, "en", 2) == 0) {
        lang = "English";
      }
      else if (strncmp (lang, "de", 2) == 0) {
        lang = "German";
      }
      else if (strncmp (lang, "fr", 2) == 0) {
        lang = "French";
      }
      else if (strncmp (lang, "sl", 2) == 0) {
        lang = "Slovene";
      }
      else {
        lang = "Unknown";
      }
      len = (uint32_t)sprintf (buf, &env[2], lang, netHTTPs_GetLanguage());
      break;

    case 'f':
      // LCD Module control from 'lcd.cgi'
			lcd_stop = true;
      switch (env[2]) {
        case '1':
          len = (uint32_t)sprintf (buf, &env[4], lcd_text[0]);
          break;
        case '2':
          len = (uint32_t)sprintf (buf, &env[4], lcd_text[1]);
          break;
      }
      break;

     case 'g':
      // AD Input from 'control.cgi'
      switch (env[2]) {
        case '1':
          len = (uint32_t)sprintf (buf, &env[4], consumo_S );
          break;
        case '2':
          len = (uint32_t)sprintf (buf, &env[4], direccion_S );
          break;
				case '3':
          len = (uint32_t)sprintf (buf, &env[4], marcha_S);
          break;
        case '4':
          len = (uint32_t)sprintf (buf, &env[4], distancia_S);
          break;
        case '5':
          len = (uint32_t)sprintf (buf, &env[4], hora);
          break;
        case '6':
          len = (uint32_t)sprintf (buf, &env[4], fecha);
          break;				
      }
      break;

       case 'x':
      len = (uint32_t)sprintf (buf, &env[1], distancia_S);
      break;
		case 'h':
      len = (uint32_t)sprintf (buf, &env[1], consumo_S);
      break;
    case 'k':  
			len = (uint32_t)sprintf (buf, &env[1], marcha_S);		
      break;			
		case 'm':  
			len = (uint32_t)sprintf (buf, &env[1], direccion_S);
      break;
    case 'y':
			len = sprintf(buf, &env[1], hora);
			break;				
		case 'w':
			len = sprintf(buf, &env[1], fecha);
			break;

//    case 'h':
//       // RTC Input from 'rtc.cgi'
//    switch (env[2]) {
//      case '1':
//        len = (uint32_t)sprintf (buf, &env[4], rtc_date_time[0]);
//      break;
//      case '2':
//        len = (uint32_t)sprintf (buf, &env[4], rtc_date_time[1]);
//      break;
//    }
		
//			case 'i':
//				//RTC Input from 'rtc.cgx'
//			switch (env[2]) {
//				case '1':
//					len = (uint32_t)sprintf (buf, &env[4], rtc_date_time[0]);
//				break;
//				case '2':
//					len = (uint32_t)sprintf (buf, &env[4], rtc_date_time[1]);
//      break;
//    }
//			break;
			
//    case 'x':
//      // AD Input from 'ad.cgx'
//      adv = AD_in (0);
//      len = (uint32_t)sprintf (buf, &env[1], adv);
//      break;

//    case 'y':
//      // Button state from 'button.cgx'
//      //len = (uint32_t)sprintf (buf, "<checkbox><id>button%c</id><on>%s</on></checkbox>",
//      //                         env[1], (get_button () & (1 << (env[1]-'0'))) ? "true" : "false");
//      break;
     case 'i':
        switch (env[2]) {
        case '0':
          snprintf(hora_flash_S, sizeof(hora_flash_S), "%.8s", horas_consumo[0]); // Limita a 8 caracteres
          len = (uint32_t)sprintf (buf, &env[4], hora_flash_S );
          break;
        case '1':
          snprintf(hora_flash_S, sizeof(hora_flash_S), "%.8s", horas_consumo[1]); // Limita a 8 caracteres
          len = (uint32_t)sprintf (buf, &env[4], hora_flash_S );
          break;
				case '2':
          snprintf(hora_flash_S, sizeof(hora_flash_S), "%.8s", horas_consumo[2]); // Limita a 8 caracteres
          len = (uint32_t)sprintf (buf, &env[4], hora_flash_S );
          break;
        case '3':
          snprintf(hora_flash_S, sizeof(hora_flash_S), "%.8s", horas_consumo[3]); // Limita a 8 caracteres
          len = (uint32_t)sprintf (buf, &env[4], hora_flash_S );
          break;
        case '4':
          snprintf(hora_flash_S, sizeof(hora_flash_S), "%.8s", horas_consumo[4]); // Limita a 8 caracteres
          len = (uint32_t)sprintf (buf, &env[4], hora_flash_S );
          break;
        case '5':
          snprintf(hora_flash_S, sizeof(hora_flash_S), "%.8s", horas_consumo[5]); // Limita a 8 caracteres
          len = (uint32_t)sprintf (buf, &env[4], hora_flash_S );
          break;
				case '6':
          snprintf(hora_flash_S, sizeof(hora_flash_S), "%.8s", horas_consumo[6]); // Limita a 8 caracteres
          len = (uint32_t)sprintf (buf, &env[4], hora_flash_S );
          break;
        case '7':
          snprintf(hora_flash_S, sizeof(hora_flash_S), "%.8s", horas_consumo[7]); // Limita a 8 caracteres
          len = (uint32_t)sprintf (buf, &env[4], hora_flash_S );
          break;
        case '8':
          snprintf(hora_flash_S, sizeof(hora_flash_S), "%.8s", horas_consumo[8]); // Limita a 8 caracteres
          len = (uint32_t)sprintf (buf, &env[4], hora_flash_S );
          break;
        case '9':
          snprintf(hora_flash_S, sizeof(hora_flash_S), "%.8s", horas_consumo[9]); // Limita a 8 caracteres
          len = (uint32_t)sprintf (buf, &env[4], hora_flash_S );
        break;
      }
      break;
     case 'z':
        switch (env[2]) {
        case '0':
          sprintf(consumo_flash_S,"%.2f mA", medidas_consumo[0]);
          len = (uint32_t)sprintf (buf, &env[4], consumo_flash_S );
          break;
        case '1':
          sprintf(consumo_flash_S,"%.2f mA", medidas_consumo[1]);
          len = (uint32_t)sprintf (buf, &env[4], consumo_flash_S );
          break;
				case '2':
          sprintf(consumo_flash_S,"%.2f mA", medidas_consumo[2]);
          len = (uint32_t)sprintf (buf, &env[4], consumo_flash_S );
          break;
        case '3':
          sprintf(consumo_flash_S,"%.2f mA", medidas_consumo[3]);
          len = (uint32_t)sprintf (buf, &env[4], consumo_flash_S );
          break;
        case '4':
          sprintf(consumo_flash_S,"%.2f mA", medidas_consumo[4]);
          len = (uint32_t)sprintf (buf, &env[4], consumo_flash_S );
          break;
        case '5':
          sprintf(consumo_flash_S,"%.2f mA", medidas_consumo[5]);
          len = (uint32_t)sprintf (buf, &env[4], consumo_flash_S );
          break;
        case '6':
          sprintf(consumo_flash_S,"%.2f mA", medidas_consumo[6]);
          len = (uint32_t)sprintf (buf, &env[4], consumo_flash_S );
          break;
        case '7':
          sprintf(consumo_flash_S,"%.2f mA", medidas_consumo[7]);
          len = (uint32_t)sprintf (buf, &env[4], consumo_flash_S );
          break;
        case '8':
          sprintf(consumo_flash_S,"%.2f mA", medidas_consumo[8]);
          len = (uint32_t)sprintf (buf, &env[4], consumo_flash_S );
          break;
        case '9':
          sprintf(consumo_flash_S,"%.2f mA", medidas_consumo[9]);
          len = (uint32_t)sprintf (buf, &env[4], consumo_flash_S );
          break;
      }
      break;
  }
  return (len);
}

#if      defined (__ARMCC_VERSION) && (__ARMCC_VERSION >= 6010050)
#pragma  clang diagnostic pop
#endif
