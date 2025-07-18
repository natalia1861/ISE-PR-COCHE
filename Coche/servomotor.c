#include "servomotor.h"
#include "cmsis_os2.h"
#include "app_main.h"

 /*
 Datos tecnicos:
 - El servomor funciona con una frecuencia de 50Hz
 - Para controlarlo, se va cambiando el periodo del pulso
Servomotor de direccion (180) -> Izquierda - 2ms (180 grados) Centro - 1.5ms (90 grados) Izquierda 1ms (0 grados)
Servomotor de velocidad (360) -> Girar hacia un lado max -> 2ms. Parado -> 1.5ms. Girar hacia el otro lado max -> 1ms

 */
 
//valores minimos y maximos que debera tener el duty segun el period configurado en PWM
#define MIN_DIRECTION_PWM               1000    //Estos valores los obtenemos tras configurar el timer, ver funcion initTim1PWM
#define MAX_DIRECTION_PWM               2000
#define MIDDLE_DIRECTION_PWM            ((MIN_DIRECTION_PWM + MAX_DIRECTION_PWM)/2)     //1500

//Restriccion para el servomotor delantero (para que no haga un giro de 90 grados y el coche se quede chueco)
#define ANGULAR_RESTR_PWM               250

//Desde el iman se recibe un angulo entre 0-360 grados.
//El servo, fisicamente funciona hasta 180 grados. En un rango de 1000-2000 en pulse.
//Logicamente, se va a limitar entre 135 y 225 (90 grados) para evitar un giro excesivo.
#define MIDDLE_ANGLE                    180                               //Angulo recibido desde web que implica la mitad
#define MIN_ANGLE_LIMIT                 (MIDDLE_ANGLE - MIDDLE_ANGLE/3)   //Minimo angulo limitado 135  
#define MAX_ANGLE_LIMIT                 (MIDDLE_ANGLE + MIDDLE_ANGLE/3)   //Maximo angulo limitado 225
#define DIRECTION_ANGLE_RANGE           180   //El rango del servo sigue siendo de 180 grados (no de 90 grados como se limita de manera logica)


#define DIRECTION_SENSIBILITY           2   //2 grados = THRESHOLD en mando para enviar datos
#define COMPLETE_DIRECTION_RANGE        180

//Valores que significa cada max y min de PWM respecto a la velocidad - 2 marchas posibles. 0 - no velocidad
#define MIN_VELOCITY                    SM_MARCHA_0
#define MAX_VELOCITY                    SM_MARCHA_2

//PWM
static TIM_HandleTypeDef htim1;             // Estructura para el TIM1 en modo PWM
static TIM_HandleTypeDef htim3;             // Estructura para el TIM3 en modo PWM
static TIM_HandleTypeDef htim4;
static TIM_OC_InitTypeDef sConfigOC;        //config tim1 PWM mode
static GPIO_InitTypeDef GPIO_InitStruct;    //estructura config pin salida TIM1

//internal functions
static void initTim1PWM(void);
static void initPinPE9(void);
static void initTim3PWM(void);
static void initPinPA6(void);
static void initTim4PWM(void);
static void initPinPD15(void);

void Init_Servomotors (void)
{
    //Init servomotor de direccion
    initTim1PWM();
	  initPinPE9();

    //Init servomotor de velocidad rueda izquierda
    initTim3PWM();
    initPinPA6();

    //Init servomotor de velocidad rueda derecha
    initTim4PWM();
    initPinPD15();
}

void DeInit_Servomotors(void)
{
    // Detener PWM de los servomotores de direcci�n (TIM1)
    HAL_TIM_PWM_Stop(&htim1, TIM_CHANNEL_1);
    __HAL_RCC_TIM1_CLK_DISABLE();  // Deshabilitar el reloj del TIM1

    // Detener PWM de los servomotores de velocidad (TIM3 y TIM4)
    HAL_TIM_PWM_Stop(&htim3, TIM_CHANNEL_1);
    __HAL_RCC_TIM3_CLK_DISABLE();  // Deshabilitar el reloj del TIM3

    HAL_TIM_PWM_Stop(&htim4, TIM_CHANNEL_4);
    __HAL_RCC_TIM4_CLK_DISABLE();  // Deshabilitar el reloj del TIM4

    // Deshabilitar los pines de salida
    HAL_GPIO_DeInit(GPIOE, GPIO_PIN_9);  // Deshabilitar pin PE9 (TIM1)
    HAL_GPIO_DeInit(GPIOA, GPIO_PIN_6);  // Deshabilitar pin PA6 (TIM3)
    HAL_GPIO_DeInit(GPIOD, GPIO_PIN_15); // Deshabilitar pin PD15 (TIM4)
}

/*
Funcion que inicializa el timer 1 para PWM
APB2 = 168MHz - SystemCoreClock
*/
static void initTim1PWM(void) {
  __HAL_RCC_TIM1_CLK_ENABLE();
  htim1.Instance = TIM1;
  htim1.Init.Prescaler = 167;    //168MHz/168=1M
  htim1.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim1.Init.Period = 19999;     //1M/20k = 50Hz
  htim1.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
  HAL_TIM_PWM_Init(&htim1);
  
  //Como el period es 20k, pulse valdra entre 0 y 20k
  //50Hz = 20ms
  //20ms/20k = 1us cada numero de pulse
  //Para conseguir: 1ms, pulse tendra que valer 1m/1us = 1000				- izquierda
  //                1.5 ms, pulse tendra que valer 1.5m/1us = 1500
  //                2 ms, pulse tendra que valer 2m/1us = 2000					- derecha
  sConfigOC.OCMode = TIM_OCMODE_PWM1;
  sConfigOC.Pulse = MIDDLE_DIRECTION_PWM;   //empezamos en la mitad del angulo
  sConfigOC.OCPolarity = TIM_OCPOLARITY_HIGH;
  sConfigOC.OCFastMode = TIM_OCFAST_DISABLE;
  HAL_TIM_PWM_ConfigChannel(&htim1, &sConfigOC, TIM_CHANNEL_1);
  HAL_TIM_PWM_Start(&htim1, TIM_CHANNEL_1);
}

static void initPinPE9(void){ //Pin salida PE9 TIM1
  __HAL_RCC_GPIOE_CLK_ENABLE();
  GPIO_InitStruct.Pin = GPIO_PIN_9;
  GPIO_InitStruct.Alternate = GPIO_AF1_TIM1;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
  GPIO_InitStruct.Speed = GPIO_SPEED_FAST;
  HAL_GPIO_Init(GPIOE, &GPIO_InitStruct);
}

// Funcion para establecer el angulo del servomotor
void setServoAngle(float angle) {
  // Asegurarse de que el angulo esta dentro del rango permitido
  if (angle < (MIN_ANGLE_LIMIT)) angle = MIN_ANGLE_LIMIT;   //135
  if (angle > (MAX_ANGLE_LIMIT)) angle = MAX_ANGLE_LIMIT;   //225

  // Calcular el valor del pulso PWM correspondiente al angulo (de 1000 a 2000)
  uint16_t pulse = (uint16_t) ((MIN_DIRECTION_PWM + ANGULAR_RESTR_PWM) + ((angle - MIN_ANGLE_LIMIT) * ((MAX_DIRECTION_PWM - ANGULAR_RESTR_PWM) - (MIN_DIRECTION_PWM + ANGULAR_RESTR_PWM)) / (MAX_ANGLE_LIMIT - MIN_ANGLE_LIMIT)));

  // Establecer el valor de comparacion (pulso) en el timer
  __HAL_TIM_SET_COMPARE(&htim1, TIM_CHANNEL_1, pulse);
}

/*
  Funcion que inicializa el timer 3 para PWM
  APB1 = 84MHz - SystemCoreClock
*/
static void initTim3PWM(void) {
  __HAL_RCC_TIM3_CLK_ENABLE();
  htim3.Instance = TIM3;
  htim3.Init.Prescaler = 83;    // 84MHz / 84 = 1M
  htim3.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim3.Init.Period = 19999;     // 1M / 20k = 50Hz
  htim3.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
  HAL_TIM_PWM_Init(&htim3);
  
  // Configuracion del PWM
  sConfigOC.OCMode = TIM_OCMODE_PWM1;
  sConfigOC.Pulse = MIDDLE_DIRECTION_PWM;  // Empezamos en la mitad (parada)
  sConfigOC.OCPolarity = TIM_OCPOLARITY_HIGH;
  sConfigOC.OCFastMode = TIM_OCFAST_DISABLE;
  HAL_TIM_PWM_ConfigChannel(&htim3, &sConfigOC, TIM_CHANNEL_1);
  HAL_TIM_PWM_Start(&htim3, TIM_CHANNEL_1);
}

static void initPinPA6(void){ // Pin salida PA6 TIM3
  __HAL_RCC_GPIOA_CLK_ENABLE();
  GPIO_InitStruct.Pin = GPIO_PIN_6;
  GPIO_InitStruct.Alternate = GPIO_AF2_TIM3;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
  GPIO_InitStruct.Speed = GPIO_SPEED_FAST;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);
}

/*
 * Funcion que inicializa el timer 4 para PWM
 * APB1 = 84MHz -> Prescaler 83: 84MHz / 84 = 1MHz -> 1us por tick
 */
static void initTim4PWM(void) {
  __HAL_RCC_TIM4_CLK_ENABLE();
  htim4.Instance = TIM4;
  htim4.Init.Prescaler = 83;         // 84MHz / (83+1) = 1MHz (1us por cuenta)
  htim4.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim4.Init.Period = 19999;         // 1MHz / 50Hz = 20000 -> 20ms período
  htim4.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
  HAL_TIM_PWM_Init(&htim4);

  sConfigOC.OCMode = TIM_OCMODE_PWM1;
  sConfigOC.Pulse = MIDDLE_DIRECTION_PWM;  // valor inicial del pulso
  sConfigOC.OCPolarity = TIM_OCPOLARITY_HIGH;
  sConfigOC.OCFastMode = TIM_OCFAST_DISABLE;
  HAL_TIM_PWM_ConfigChannel(&htim4, &sConfigOC, TIM_CHANNEL_4);
  HAL_TIM_PWM_Start(&htim4, TIM_CHANNEL_4);
}

/*
 * Configura el pin PD15 como salida PWM (TIM4_CH4)
 */
static void initPinPD15(void) {
  __HAL_RCC_GPIOD_CLK_ENABLE();
  GPIO_InitStruct.Pin = GPIO_PIN_15;
  GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FAST;
  GPIO_InitStruct.Alternate = GPIO_AF2_TIM4;  // AF2 para TIM4_CH4
  HAL_GPIO_Init(GPIOD, &GPIO_InitStruct);
}

// Funcion para establecer la velocidad del motor (usando PWM de 1000 a 2000)
/*| `speed` | `back_gear` | `pulse` |
| ------- | ----------- | ------- |
| 0       | false       | 1500    |
| 1       | false       | 1750    |
| 2       | false       | 2000    |
| 0       | true        | 1500    |
| 1       | true        | 1250    |
| 2       | true        | 1000    |
*/

void setMotorSpeed(speed_marchas_t speed) {
    // Asegurar que la velocidad esta dentro del rango permitido
    if (speed < MIN_VELOCITY) speed = MIN_VELOCITY;
    if (speed > MAX_VELOCITY) speed = MAX_VELOCITY;

    uint16_t pulse_left = MIDDLE_DIRECTION_PWM;
    uint16_t pulse_right = MIDDLE_DIRECTION_PWM;
    
    if (speed == MIN_VELOCITY) 
    {
        // Vehiculo parado
        pulse_left = MIDDLE_DIRECTION_PWM;
        pulse_right = MIDDLE_DIRECTION_PWM;
    } else {
        // Calculo lineal de PWM entre MIDDLE y MAX o MIN segun el modo
        if (app_coche_state == STATE__BACK_GEAR)  //BACK GEAR Mode
        { 
            // Reversa izquierda: MIDDLE a MIN
            // Avance derecha: MIDDLE a MAX
            pulse_left = MIDDLE_DIRECTION_PWM - ((MIDDLE_DIRECTION_PWM - MIN_DIRECTION_PWM) * speed) / (MAX_VELOCITY-MIN_VELOCITY);
            pulse_right = MIDDLE_DIRECTION_PWM + ((MAX_DIRECTION_PWM - MIDDLE_DIRECTION_PWM) * speed) / (MAX_VELOCITY-MIN_VELOCITY);
        } 
        else 
        {
            // Avance izquierda: MIDDLE a MAX
            // Reversa derecha: MIDDLE a MIN
            pulse_left = MIDDLE_DIRECTION_PWM + ((MAX_DIRECTION_PWM - MIDDLE_DIRECTION_PWM) * speed) / (MAX_VELOCITY-MIN_VELOCITY);
            pulse_right = MIDDLE_DIRECTION_PWM - ((MIDDLE_DIRECTION_PWM - MIN_DIRECTION_PWM) * speed) / (MAX_VELOCITY-MIN_VELOCITY);
        }
    }

    __HAL_TIM_SET_COMPARE(&htim3, TIM_CHANNEL_1, pulse_left); //Se actualiza la rueda izquierda
    __HAL_TIM_SET_COMPARE(&htim4, TIM_CHANNEL_4, pulse_right); //Se actualiza la rueda derecha
}

