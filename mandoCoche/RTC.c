#include "RTC.h"
#include "cmsis_os2.h"                  // ::CMSIS:RTOS2a
#include "app_main.h"

#define FLAG_RTC                0x02

osStatus_t status;
//declaraciones internas
static void RTC_Set_Time_SNTP(void);
static void get_sntp_time (void);
static void sntp_time_callback (uint32_t time, uint32_t seconds_fraction);

//estructuras
static RTC_HandleTypeDef hrtc;
static RTC_DateTypeDef fech;
static RTC_TimeTypeDef hor;

//vairables locales
struct tm horaSNTP;

//Variable de la hora y fecha para el LCD
char rtc_date_time[RTC_MAX][LCD_MAX_CHARACTERS+1];	//maximo de caracteres + EOS

//timers
static osTimerId_t tim_id_3min;	//timer sincronizacion cada 3 min

static void Timer_Callback_3min (void);
char hora[80];    //variable global usada en web
char fecha[80];   //variable global usada en web

void init_RTC(void) {     //inicializa el RTC y configura una hora por defecto
    hrtc.Instance = RTC; 
    hrtc.Init.HourFormat = RTC_HOURFORMAT_24;
    hrtc.Init.AsynchPrediv = 127;
    hrtc.Init.SynchPrediv = 255;
    hrtc.Init.OutPut = RTC_OUTPUT_DISABLE;
    hrtc.Init.OutPutPolarity = RTC_OUTPUT_POLARITY_HIGH;
    hrtc.Init.OutPutType = RTC_OUTPUT_TYPE_OPENDRAIN;
    __HAL_RTC_RESET_HANDLE_STATE(&hrtc);
    
    if (HAL_RTC_Init(&hrtc) != HAL_OK) {
        /* Initialization Error */
        printf("Initialization RTC Error");
    }
	
	//Se anade hora generica
	  RTC_set_Time(18,6,1,0,0,0);
  
  //Tras reinicar esperamos 6 segundos antes de poner la hora SNTP
    osDelay(6000);
  	get_sntp_time();
  
   //Iniciamos el timer para volver a actualizar la hora con el servidor SNTP
  tim_id_3min = osTimerNew((osTimerFunc_t)&Timer_Callback_3min, osTimerPeriodic, NULL, NULL);
	if (osTimerStart(tim_id_3min, 180000) != osOK)
    {
      //revisar anadir error
        printf("Error Timer RTC");
    }
}

//Anade una hora que se le especifica
void RTC_set_Time(uint8_t day, uint8_t month, uint8_t year, uint8_t hora, uint8_t minutos, uint8_t segundos) {  
	fech.Year=year;
	fech.Month=month;
	fech.Date=day;
	HAL_RTC_SetDate(&hrtc,&fech,RTC_FORMAT_BIN);
	
	hor.Hours=hora;
	hor.Minutes=minutos;
    hor.Seconds=segundos;
    hor.TimeFormat=RTC_HOURFORMAT_24;
	hor.DayLightSaving = RTC_DAYLIGHTSAVING_NONE ;
    hor.StoreOperation = RTC_STOREOPERATION_RESET;
	HAL_RTC_SetTime(&hrtc,&hor,RTC_FORMAT_BIN);
}

//Recibe la fecha y la hora y lo escribe en variables globales
void RTC_getTime_Date(void) {   
  RTC_DateTypeDef sdatestructureget;
  RTC_TimeTypeDef stimestructureget;
  HAL_RTC_GetTime(&hrtc,&stimestructureget,RTC_FORMAT_BIN);
  HAL_RTC_GetDate(&hrtc,&sdatestructureget,RTC_FORMAT_BIN);
  
  //Se actualiza hora y fecha en lcd
  sprintf(rtc_date_time[0],"%02d:%02d:%02d",stimestructureget.Hours, stimestructureget.Minutes,stimestructureget.Seconds);
  sprintf(rtc_date_time[1],"%02d/%02d/%02d",sdatestructureget.Date, sdatestructureget.Month,2000+sdatestructureget.Year);
  osThreadFlagsSet (id_thread__app_main, FLAG__MOSTRAR_HORA);

  //Se actualiza hora y fecha en web
  sprintf(hora,"%02d:%02d:%02d",stimestructureget.Hours, stimestructureget.Minutes,stimestructureget.Seconds);
  sprintf(fecha,"%02d/%02d/%02d",sdatestructureget.Date, sdatestructureget.Month,2000+sdatestructureget.Year);
}

void HAL_RTC_MspInit(RTC_HandleTypeDef *hrtc) {
  RCC_OscInitTypeDef        RCC_OscInitStruct;
  RCC_PeriphCLKInitTypeDef  PeriphClkInitStruct;

  /*##-1- Enables the PWR Clock and Enables access to the backup domain ###################################*/
  /* To change the source clock of the RTC feature (LSE, LSI), You have to:
     - Enable the power clock using __HAL_RCC_PWR_CLK_ENABLE()
     - Enable write access using HAL_PWR_EnableBkUpAccess() function before to 
       configure the RTC clock source (to be done once after reset).
     - Reset the Back up Domain using __HAL_RCC_BACKUPRESET_FORCE() and 
       __HAL_RCC_BACKUPRESET_RELEASE().
     - Configure the needed RTc clock source */
  __HAL_RCC_PWR_CLK_ENABLE();
  HAL_PWR_EnableBkUpAccess();

  
  //##-2- Configure LSE as RTC clock source ###################################/
  RCC_OscInitStruct.OscillatorType =  RCC_OSCILLATORTYPE_LSE;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_NONE;
  RCC_OscInitStruct.LSEState = RCC_LSE_ON;
  if(HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK) { 
    //Error_Handler();
  }
  
  PeriphClkInitStruct.PeriphClockSelection = RCC_PERIPHCLK_RTC;
  PeriphClkInitStruct.RTCClockSelection = RCC_RTCCLKSOURCE_LSE;
  if(HAL_RCCEx_PeriphCLKConfig(&PeriphClkInitStruct) != HAL_OK) { 
    //Error_Handler();
  }
  
  //##-3- Enable RTC peripheral Clocks #######################################/
  /* Enable RTC Clock */
  __HAL_RCC_RTC_ENABLE();
}

//SNTP
//Funcion que actualiza el RTC con la hora del SNTP
static void RTC_Set_Time_SNTP(void) {
	fech.Year=horaSNTP.tm_year+1900-2000;
	fech.Month=horaSNTP.tm_mon+1;
	fech.Date=horaSNTP.tm_mday;
	HAL_RTC_SetDate(&hrtc,&fech,RTC_FORMAT_BIN);
	hor.Hours=horaSNTP.tm_hour+1;
	hor.Minutes=horaSNTP.tm_min;
    hor.Seconds=horaSNTP.tm_sec;
    hor.TimeFormat=RTC_HOURFORMAT_24;
	hor.DayLightSaving = RTC_DAYLIGHTSAVING_NONE ;
    hor.StoreOperation = RTC_STOREOPERATION_RESET;
	HAL_RTC_SetTime(&hrtc,&hor,RTC_FORMAT_BIN);
}

//Funcion que obtiene la hora SNTP
static void get_sntp_time (void) {
	//LED_Rrun = true;
	//osTimerStart(tim_id_2seg, 2000U);
	netSNTPc_GetTime (NULL, sntp_time_callback);
}

//Callback del servidor SNTP
static void sntp_time_callback (uint32_t time, uint32_t seconds_fraction) {
	uint32_t hora;
	if (time==0) {
		//ERROR
        printf("Error SNTP");
	} else {
		hora = time;
		horaSNTP = *localtime(&hora);
		RTC_Set_Time_SNTP();
	}
}

//Cada 3min se sincroniza la hora con el servidor SNTP
static void Timer_Callback_3min (void) {
	 get_sntp_time();							
}

//HILO RTC
static void thread__RTC_Update (void *no_argument) {
	while (1) 
    {
        /* Every 1000 ms */
        RTC_getTime_Date();
		
        osDelay (1000);
	}
}

void Init_RTC_Update (void)
{
    init_RTC();
    osThreadNew(thread__RTC_Update, NULL, NULL);
}
