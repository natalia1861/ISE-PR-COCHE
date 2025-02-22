#include "RTC.h"

#define FLAG_RTC 0x02

osStatus_t status;
//declaraciones internas
static void RTC_Set_Time_SNTP(void);
static void get_sntp_time (void);
static void sntp_time_callback (uint32_t time, uint32_t seconds_fraction);

//estructuras
static RTC_HandleTypeDef hrtc;
static RTC_DateTypeDef fech;
static RTC_TimeTypeDef hor;
static RTC_AlarmTypeDef alarma; 


//vairables locales
static int alarma_segundos;
struct tm horaSNTP;

//variables externas
extern osThreadId_t TID_Led; //referenciado de HTTP_Server.c
extern osThreadId_t TID_Display; //referenciado de HTTP_Server.c
extern char rtc_date_time[2][20+1];	//referenciado a HTTP_Server.c

//control de leds
extern bool LED_Rrun;	//referenciado a HTTP_Server.c
extern bool LED_Grun;	//referenciado a HTTP_Server.c

//timers
static osTimerId_t tim_id_2seg;	//led rojo parpadea durante 2 seg
static osTimerId_t tim_id_verde;	//led verde parpadea durante 5 seg
static osTimerId_t tim_id_6seg;	//se aconseja esperar al menos 5 seg tras la primera sincronizacion
static osTimerId_t tim_id_3min;	//timer sincronizacion cada 3 min

static void Timer_Callback_2seg (void);
static void Timer_Callback_verde (void);
static void Timer_Callback_6seg(void);
static void Timer_Callback_3min (void);

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
   // Error_Handler();
  }
//  if (HAL_RTCEx_BKUPRead(&hrtc, RTC_BKP_DR1) != 0x32F2) {
//    /* Configure RTC Calendar */
//      RTC_set_Time();
//  } else {
//    /* Check if the Power On Reset flag is set */
//    if (__HAL_RCC_GET_FLAG(RCC_FLAG_PORRST) != RESET) {
//      /* Turn on LED2: Power on reset occurred */
//    //  BSP_LED_On(LED2);
//    }
//    /* Check if Pin Reset flag is set */
//    if (__HAL_RCC_GET_FLAG(RCC_FLAG_PINRST) != RESET)
//    {
//      /* Turn on LED1: External reset occurred */
//    //  BSP_LED_On(LED1);
//    }
//    /* Clear source Reset Flag */
//   // __HAL_RCC_CLEAR_RESET_FLAGS();
//  }
	//HAL_RTC_Init(&hrtc);
  //CONFIGURACION ALARMA
//	alarma.Alarm=RTC_ALARM_A;
//	alarma.AlarmTime.Seconds=0;
//	HAL_RTC_SetAlarm_IT(&hrtc,&alarma,RTC_FORMAT_BIN);
//	
//	HAL_NVIC_SetPriority(RTC_Alarm_IRQn,0,0);

	//Init_alarma(alarma_segundos);
	//RTC_getTime_Date();
	
	//TRAS REINICIAR HAY QUE ESPERAR 6 SEG PARA SINCRONIZAR CON EL SERVICIO SNTP
	
	RTC_set_Time(18,6,1,0,0,0);
	Init_alarma(5);
	tim_id_6seg = osTimerNew((osTimerFunc_t)&Timer_Callback_6seg, osTimerOnce, NULL, NULL);
	tim_id_3min = osTimerNew((osTimerFunc_t)&Timer_Callback_3min, osTimerPeriodic, NULL, NULL);
	tim_id_verde = osTimerNew((osTimerFunc_t)&Timer_Callback_verde, osTimerOnce, NULL, NULL);
	tim_id_2seg = osTimerNew((osTimerFunc_t)&Timer_Callback_2seg, osTimerOnce, NULL, NULL);
	osTimerStart(tim_id_6seg, 6000);
	
}
  

void RTC_set_Time(uint8_t day, uint8_t month, uint8_t year, uint8_t hora, uint8_t minutos, uint8_t segundos) {   //pone la hora a enero las 20:20:20
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

void RTC_getTime_Date(void) {   //recibe la fecha y la hora y lo escribe por las lineas del lcd
  RTC_DateTypeDef sdatestructureget;
  RTC_TimeTypeDef stimestructureget;
  HAL_RTC_GetTime(&hrtc,&stimestructureget,RTC_FORMAT_BIN);
  HAL_RTC_GetDate(&hrtc,&sdatestructureget,RTC_FORMAT_BIN);
  
  sprintf(rtc_date_time[0],"%.2d:%.2d:%.2d",stimestructureget.Hours, stimestructureget.Minutes,stimestructureget.Seconds);
  sprintf(rtc_date_time[1],"%.2d/%.2d/%.2d",sdatestructureget.Date, sdatestructureget.Month,2000+sdatestructureget.Year);
  osThreadFlagsSet (TID_Display, FLAG_RTC);
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

void HAL_RTC_AlarmAEventCallback(RTC_HandleTypeDef *hrtc) {
	LED_Grun = true;
	//puedes poner un flag o variable global, pero timers no
	
	//Init_alarma(10);	//permite configurar a que segundos queremos la alarma
}

void RTC_Alarm_IRQHandler(void) {
  HAL_RTC_AlarmIRQHandler(&hrtc);
}

void Init_alarma (int seg_alarm){
	HAL_NVIC_EnableIRQ(RTC_Alarm_IRQn);
	alarma.Alarm = RTC_ALARM_A;
	alarma.AlarmTime.TimeFormat = RTC_HOURFORMAT12_AM;
	alarma.AlarmTime.DayLightSaving = RTC_DAYLIGHTSAVING_NONE;
	alarma.AlarmTime.StoreOperation = RTC_STOREOPERATION_SET;

	alarma.AlarmMask = RTC_ALARMMASK_DATEWEEKDAY|RTC_ALARMMASK_HOURS|RTC_ALARMMASK_MINUTES;
	alarma.AlarmDateWeekDaySel = RTC_ALARMDATEWEEKDAYSEL_DATE;
	alarma.AlarmDateWeekDay = 1;
	alarma.AlarmTime.Seconds = seg_alarm;
	
	HAL_RTC_SetAlarm_IT(&hrtc, &alarma, RTC_FORMAT_BIN);
	
}
//SNTP
static void RTC_Set_Time_SNTP(void) {	//pone la hora del servidor de sntp
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

static void get_sntp_time (void) {
	LED_Rrun = true;
	osTimerStart(tim_id_2seg, 2000U);
	netSNTPc_GetTime (NULL, sntp_time_callback);
}

static void sntp_time_callback (uint32_t time, uint32_t seconds_fraction) {
	uint32_t hora;
	if (time==0) {
		//ERROR
	} else {
		hora = time;
		horaSNTP = *localtime(&hora);
		RTC_Set_Time_SNTP();
	}
}

//timer callbacks
static void Timer_Callback_verde(void){
	 LED_Grun = false;	//apagamos el led verde
}

static void Timer_Callback_3min (void) {	//cada 3 min se sincroniza con sntp
	 get_sntp_time();								//el led rojo parpadea durante 2 seg
}
 
static void Timer_Callback_2seg (void) {
	 LED_Rrun = false;	//apagamos el led rojo
}
 
static void Timer_Callback_6seg (void) {
	get_sntp_time();
	status = osTimerStart(tim_id_verde, 5000U);
	osTimerStart(tim_id_3min, 180000);
}

