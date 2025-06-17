#include "cmsis_os2.h"                          // CMSIS RTOS header file
#include "stm32f4xx_hal.h"
#include "alarma_control.h"
#include <stdio.h>
#include <string.h>
#include "errors.h"

/*----------------------------------------------------------------------------
 *      												Thread PWM
 *---------------------------------------------------------------------------*/

#define PWM_TONE_FREQUENCY		2000		//Hz

#define BEEP_LENTO_MS     		800			//ms
#define BEEP_MEDIO_MS     		400			//ms
#define BEEP_RAPIDO_MS    		100			//ms

//Este PWM esta empleado para el pitido

//TIMER FISICO
static TIM_HandleTypeDef htim3; 
static TIM_OC_InitTypeDef sConfigOC;

//Thread
osThreadId_t id_thread__AlarmaControl = NULL;

//Timer virtual
static osTimerId_t id_timer__AlarmaTono = NULL;
static void Timer__AlarmaTono (void *no_argument);

void PWM_AlarmInit(void);

/*
  Funcion que inicializa el timer 3 para PWM con salida a pin PC8
  APB1 = 84MHz - SystemCoreClock
*/

void Init_PWM_Alarma(void)
{
	GPIO_InitTypeDef GPIO_InitStruct={0};
	__HAL_RCC_GPIOC_CLK_ENABLE();

  //PIN PC8
	GPIO_InitStruct.Pin=GPIO_PIN_8;
	GPIO_InitStruct.Mode=GPIO_MODE_AF_PP; 
	GPIO_InitStruct.Pull=GPIO_PULLDOWN;
	GPIO_InitStruct.Alternate=GPIO_AF2_TIM3; 
	HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);
	
  //Timer 3
  	__HAL_RCC_TIM3_CLK_ENABLE(); 
  	htim3.Instance = TIM3;
	htim3.Init.Prescaler = 83;               // 84 MHz / (83+1) = 1 MHz
	htim3.Init.Period = 499;                 // 1 MHz / (499+1) = 2 kHz
  	htim3.Init.CounterMode = TIM_COUNTERMODE_UP;
  	htim3.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
  	HAL_TIM_PWM_Init(&htim3);
  
  //Timer 3 -> APB1 = 84MHz - SystemCoreClock
  	sConfigOC.OCMode=TIM_OCMODE_PWM1;
  	sConfigOC.OCPolarity = TIM_OCPOLARITY_HIGH;
  	sConfigOC.OCFastMode = TIM_OCFAST_DISABLE;
	sConfigOC.Pulse = (htim3.Init.Period + 1) / 2;  // 50% duty = 250
	HAL_TIM_PWM_ConfigChannel(&htim3, &sConfigOC, TIM_CHANNEL_3);
	HAL_TIM_PWM_Start(&htim3, TIM_CHANNEL_3);
}

void PWM_AlarmDeactivate ()
{
	HAL_TIM_PWM_Stop(&htim3, TIM_CHANNEL_3);
}

void PWM_AlarmActivate ()
{
	HAL_TIM_PWM_Start(&htim3, TIM_CHANNEL_3);
}

void thread__AlarmaControl(void *no_argument)
{
	uint32_t flags;
	bool tono_on = false;

	//Inicializamos timer para activar/ desactivar tono
	while(1)
	{
		flags = osThreadFlagsWait(FLAGS_ALARMA, osFlagsWaitAny, osWaitForever);

		if (flags & FLAG_DIST_ALTA)
		{
		    if (osTimerStart(id_timer__AlarmaTono, BEEP_LENTO_MS) != osOK)
			{
				push_error(MODULE__ALARM, ERR_CODE__TIMER, 2);
			}
		}

		if (flags & FLAG_DIST_MEDIA)
		{
		    if (osTimerStart(id_timer__AlarmaTono, BEEP_MEDIO_MS) != osOK)
			{
				push_error(MODULE__ALARM, ERR_CODE__TIMER, 3);
			}	
		}	

		if (flags & FLAG_DIST_BAJA)
		{
		    if (osTimerStart(id_timer__AlarmaTono, BEEP_RAPIDO_MS) != osOK)
			{
				push_error(MODULE__ALARM, ERR_CODE__TIMER, 4);
			}	
		}

		if (flags & FLAG_DEACTIVATE_ALARM)		//Desactivar alarma de control
		{
			PWM_AlarmDeactivate();
			tono_on = false;
		}
		if (flags & FLAG_TONO)	//Cambiar de estado entre tono y no tono
		{
			if (tono_on)
			{
				PWM_AlarmDeactivate();
				tono_on = false;
			}
			else
			{
				PWM_AlarmActivate();
				tono_on = true;
			}
		}
	}
}

void Timer__AlarmaTono(void *no_argument)
{
	osThreadFlagsSet(id_thread__AlarmaControl, FLAG_TONO);
}

void Init_AlarmaControl (void)
{
	Init_PWM_Alarma();

	//Se crea el hilo de control
	if (id_thread__AlarmaControl == NULL)
		id_thread__AlarmaControl = osThreadNew(thread__AlarmaControl, NULL, NULL);

	//Se crea el timer de Control
	if (id_timer__AlarmaTono == NULL)
		id_timer__AlarmaTono = osTimerNew(Timer__AlarmaTono, osTimerOnce, NULL, NULL);
	
	if (id_thread__AlarmaControl == NULL)
	{
		push_error(MODULE__ALARM, ERR_CODE__THREAD, 0);
	}

	if (id_timer__AlarmaTono == NULL)
	{
		push_error(MODULE__ALARM, ERR_CODE__TIMER, 0);
	}

	if (osTimerStart(id_timer__AlarmaTono, BEEP_LENTO_MS) != osOK)
	{
		push_error(MODULE__ALARM, ERR_CODE__TIMER, 1);
	}
}

void Deinit_AlarmaControl (void)
{
	if (id_thread__AlarmaControl != NULL)
    {
		osThreadTerminate(id_thread__AlarmaControl);
        id_thread__AlarmaControl = NULL;
    }

	if (id_timer__AlarmaTono != NULL)
    {
		osTimerDelete(id_timer__AlarmaTono);
        id_timer__AlarmaTono = NULL;
    }
}
