#include "main.h"
#include "string.h"
#include "stdio.h"
#include "lcd.h"
#include "Arial12x12.h"

#define PIN_RESET   GPIO_PIN_6 /*PA6*/
#define PORT_RESET  GPIOA

#define PIN_A0      GPIO_PIN_13 /*PF13*/
#define PORT_A0     GPIOF

#define PIN_CS      GPIO_PIN_14 /*PD14*/
#define PORT_CS     GPIOD

extern ARM_DRIVER_SPI Driver_SPI1;
ARM_DRIVER_SPI *SPIDrv = &Driver_SPI1;

char mensajeL1[100];
char mensajeL2[100];
   uint32_t positionL1;
   uint32_t positionL2;

unsigned char buffer[512];
int i;

//Declaracion de funciones internas
void LCD_init(void);
static void LCD_reset(void);
static void LCD_update(void);

void delay(uint32_t n_microsegundos);
void LCD_wr_data(unsigned char data);
void LCD_wr_cmd(unsigned char cmd);

void vaciarLCD_L1(void);
void vaciarLCD_L2(void);
void vaciarLCD_L3(void);
void vaciarLCD_L4(void);


void LCD_start(void) {
  LCD_reset();
  LCD_init();
  LCD_update();
}

/*---------------------------------------------------
 *      			Inicialización del LCD
 *---------------------------------------------------*/

void LCD_init(void){
  LCD_wr_cmd(0xAE);
  LCD_wr_cmd(0xA2);
  LCD_wr_cmd(0xA0);
  LCD_wr_cmd(0xC8);
  LCD_wr_cmd(0x22);
  LCD_wr_cmd(0x2F);
  LCD_wr_cmd(0x40);
  LCD_wr_cmd(0xAF);
  LCD_wr_cmd(0x81);
  LCD_wr_cmd(0x17);
  LCD_wr_cmd(0xA4);
  LCD_wr_cmd(0xA6);
}

/*---------------------------------------------------
 *      			Reset del LCD
 *---------------------------------------------------*/
static void LCD_reset(void){
  static GPIO_InitTypeDef GPIO_InitStruct_LCD;
  /*CS*/
  __HAL_RCC_GPIOD_CLK_ENABLE();
  GPIO_InitStruct_LCD.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct_LCD.Pull = GPIO_PULLUP;
  GPIO_InitStruct_LCD.Speed = GPIO_SPEED_FREQ_HIGH;
  GPIO_InitStruct_LCD.Pin = PIN_CS;
  HAL_GPIO_Init(PORT_CS, &GPIO_InitStruct_LCD);
  HAL_GPIO_WritePin(PORT_CS, PIN_CS, GPIO_PIN_SET);
  
  /*A0*/
  __HAL_RCC_GPIOF_CLK_ENABLE();
  GPIO_InitStruct_LCD.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct_LCD.Pull = GPIO_PULLUP;
  GPIO_InitStruct_LCD.Speed = GPIO_SPEED_FREQ_HIGH;
  GPIO_InitStruct_LCD.Pin = PIN_A0;
  HAL_GPIO_Init(PORT_A0, &GPIO_InitStruct_LCD);
  HAL_GPIO_WritePin(PORT_A0, PIN_A0, GPIO_PIN_SET);
  
  /*Reset*/
  __HAL_RCC_GPIOA_CLK_ENABLE();
  GPIO_InitStruct_LCD.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct_LCD.Pull = GPIO_PULLUP;
  GPIO_InitStruct_LCD.Speed = GPIO_SPEED_FREQ_HIGH;
  GPIO_InitStruct_LCD.Pin = PIN_RESET;
  HAL_GPIO_Init(PORT_RESET, &GPIO_InitStruct_LCD);
  HAL_GPIO_WritePin(PORT_RESET, PIN_RESET, GPIO_PIN_SET);
  
  /*SPI*/
  SPIDrv->Initialize(NULL);
  SPIDrv-> PowerControl(ARM_POWER_FULL);
  SPIDrv-> Control(ARM_SPI_MODE_MASTER | ARM_SPI_CPOL1_CPHA1 | ARM_SPI_MSB_LSB | ARM_SPI_DATA_BITS (8), 20000000);
  HAL_GPIO_WritePin(PORT_RESET, PIN_RESET, GPIO_PIN_RESET);
  delay(1);
  HAL_GPIO_WritePin(PORT_RESET, PIN_RESET, GPIO_PIN_SET);
  delay(1000);
}

/*---------------------------------------------------
 *      			Update del LCD
 *---------------------------------------------------*/

static void LCD_update(void){
  int i;	
  LCD_wr_cmd(0x00);
  LCD_wr_cmd(0x10);
  LCD_wr_cmd(0xB0);
  for(i = 0; i < 128; i++){LCD_wr_data(buffer[i]);}

  LCD_wr_cmd(0x00);
  LCD_wr_cmd(0x10);
  LCD_wr_cmd(0xB1);
  for(i = 128; i < 256; i++){LCD_wr_data(buffer[i]);}

  LCD_wr_cmd(0x00);
  LCD_wr_cmd(0x10);
  LCD_wr_cmd(0xB2);
  for(i = 256; i < 384; i++){LCD_wr_data(buffer[i]);}
  
  LCD_wr_cmd(0x00);
  LCD_wr_cmd(0x10);
  LCD_wr_cmd(0xB3);
  for(i = 384; i < 512; i++){LCD_wr_data(buffer[i]);}
}

/*---------------------------------------------------
 *      			Limpieza del LCD
 *---------------------------------------------------*/
void LCD_clean(void){
  for(int i=0; i<512; i++)
    buffer[i]=0x00;
    LCD_update();
}

/*---------------------------------------------------
 *      			Escritura y comandos del LCD
 *---------------------------------------------------*/
void LCD_wr_data(unsigned char data){
  ARM_SPI_STATUS count;
  HAL_GPIO_WritePin(GPIOD, PIN_CS, GPIO_PIN_RESET);
  HAL_GPIO_WritePin(GPIOF,PIN_A0, GPIO_PIN_SET);
  SPIDrv -> Send(&data,sizeof(data));
do{
 count=SPIDrv->GetStatus();
}while (count.busy);
  HAL_GPIO_WritePin(GPIOD, PIN_CS, GPIO_PIN_SET);
  
}

void LCD_wr_cmd(unsigned char cmd){
  ARM_SPI_STATUS count;
  HAL_GPIO_WritePin(GPIOD, PIN_CS, GPIO_PIN_RESET);
  HAL_GPIO_WritePin(GPIOF,PIN_A0, GPIO_PIN_RESET);
  SPIDrv -> Send(&cmd,sizeof(cmd));
do{
 count=SPIDrv->GetStatus();
}while (count.busy);
  HAL_GPIO_WritePin(GPIOD, PIN_CS, GPIO_PIN_SET);
  
}

/*---------------------------------------------------
 *      			Delay del LCD
 *---------------------------------------------------*/

void delay(uint32_t n_microsegundos){
  static TIM_HandleTypeDef hTim7;

  __HAL_RCC_TIM7_CLK_ENABLE();
 
  hTim7.Instance= TIM7;
  hTim7.Init.Prescaler=83; // El 84Mhz --> (168Mhz/4)*2 ---- 84Mhz/8400 =2MHz
  hTim7.Init.Period=n_microsegundos-1; //1000Hz / 500 = 2Hz = 0.5s
   
  HAL_TIM_Base_Init(&hTim7); //Configuracion del timer
  HAL_TIM_Base_Start(&hTim7); //Start the timer
 
  while(TIM7 -> CNT < n_microsegundos-1){
  }
  HAL_TIM_Base_Stop(&hTim7); //Stop the timer*/
  
}/*Fin delay*/

/*---------------------------------------------------
 *      			Escrituras lineas 1 y 2 del LCD
 *---------------------------------------------------*/
static void symbolToLocalBuffer_L1(uint8_t symbol){
uint8_t i, value1, value2;
uint16_t offset = 0;

  
  offset = 25*(symbol - ' ');
  for (i=0; i<12 ; i++){
    
    value1=Arial12x12[offset+i*2+1];
    value2=Arial12x12[offset+i*2+2];
    buffer[i + positionL1] = value1;
    buffer[i + 128 + positionL1] = value2;
  }
  positionL1 = positionL1 + Arial12x12[offset];
}

static void symbolToLocalBuffer_L2(uint8_t symbol){
  uint8_t i, value1, value2;
  uint16_t offset = 0;
  
  offset = 25 * (symbol - ' ');
  
  for(i=0 ; i<12 ; i++){
    
    value1 = Arial12x12[offset + i * 2 + 1];
    value2 = Arial12x12[offset + i * 2 + 2];
    buffer[i+256+positionL2] = value1;
    buffer[i+384+positionL2] = value2;
  }
  positionL2 = positionL2+Arial12x12[offset];
}

static void symbolToLocalBuffer(uint8_t line, uint8_t symbol){
  if (line == 1){
    symbolToLocalBuffer_L1(symbol);}
  if (line == 2){
    symbolToLocalBuffer_L2(symbol);}
}

/*---------------------------------------------------
 *      Escribir char en la linea1 o 2 del LCD
 *---------------------------------------------------*/

void LCD_write(uint8_t line, char a[]){
  static int n;
    
  if (line == LCD_LINE__ONE)
  {
      vaciarLCD_L1();
      vaciarLCD_L2();
  }
  else  //LCD_LINE__TWO
  {
      vaciarLCD_L3();
      vaciarLCD_L4();
  }
  for(n = 0; n < strlen(a); n++){
    symbolToLocalBuffer(line, a[n]);
  }
  positionL1 = 0;
  positionL2 = 0;
  LCD_update();
}

/*---------------------------------------------------------
 * Funciones para escribir las lineas en modo marcha atrás
 *---------------------------------------------------------*/

void LCD_update_L1(void){
int i;
 LCD_wr_cmd(0x00); // 4 bits de la parte baja de la dirección a 0
 LCD_wr_cmd(0x10); // 4 bits de la parte alta de la dirección a 0
 LCD_wr_cmd(0xB0); // Página 0

 for(i=0;i<128;i++){
 LCD_wr_data(buffer[i]);
	 
 }
}
 
void LCD_update_L2(void){
	int i; 
 LCD_wr_cmd(0x00); // 4 bits de la parte baja de la dirección a 0
 LCD_wr_cmd(0x10); // 4 bits de la parte alta de la dirección a 0
 LCD_wr_cmd(0xB1); // Página 1
 
 for(i=128;i<256;i++){
 LCD_wr_data(buffer[i]);
 }
}

void LCD_update_L3(void){
 int i;	

 LCD_wr_cmd(0x00);
 LCD_wr_cmd(0x10);
 LCD_wr_cmd(0xB2); //Página 2
	
	
 for(i=256;i<384;i++){
 LCD_wr_data(buffer[i]);
 }
}

void LCD_update_L4(void){
	int i;
 LCD_wr_cmd(0x00);
 LCD_wr_cmd(0x10);
 LCD_wr_cmd(0xB3); // Pagina 3


 for(i=384;i<512;i++){
 LCD_wr_data(buffer[i]);
 }
}

void rellenarLCD_L1(void){
	for(int i= 0; i<128; i++){
    buffer[i] = 0xFF;
  }
	LCD_update_L1();
}

void rellenarLCD_L2(void){
	for(int i= 144; i<240; i++){
    buffer[i] = 0xFF;
  }
	LCD_update_L2();
}
void rellenarLCD_L3(void){
	for(int i= 280; i<360; i++){
    buffer[i] = 0xFF;
  }
	LCD_update_L3();
}
void rellenarLCD_L4(void){
	for(int i= 384; i<512; i++){
    buffer[i] = 0xAA;
  }
	LCD_update_L4();
}

void vaciarLCD_L1(void){
	for(int i= 0; i<128; i++){
    buffer[i] = 0x00;
  }
	LCD_update_L1();
}

void vaciarLCD_L2(void){
	for(int i= 128; i<256; i++){
    buffer[i] = 0x00;
  }
	LCD_update_L2();
}
void vaciarLCD_L3(void){
	for(int i= 256; i<384; i++){
    buffer[i] = 0x00;
  }
	LCD_update_L3();
}
void vaciarLCD_L4(void){
	for(int i= 384; i<512; i++){
    buffer[i] = 0x00;
  }
	LCD_update_L4();
}

void LCD_mostrarLineasDistancia (lineas_distancia_t lineas)
{
    switch (lineas)
    {
        case LCD_LINE__NO_LINE:
            vaciarLCD_L1();
            vaciarLCD_L2();
            vaciarLCD_L3();
            rellenarLCD_L4();
            break;
        case LCD_LINE__ONE:
            rellenarLCD_L1();
            vaciarLCD_L2();
            vaciarLCD_L3();
            rellenarLCD_L4();
            break;
        case LCD_LINE__TWO:
            rellenarLCD_L1();
            rellenarLCD_L2();
            vaciarLCD_L3();
            rellenarLCD_L4();
            break;
        case LCD_LINE__THREE:
            rellenarLCD_L1();
            rellenarLCD_L2();
            rellenarLCD_L3();
            rellenarLCD_L4();
            break;
        case LCD_LINE__MAX:
            printf("Error: numero de lineas mayor que el maximo permitido");
            break;
    }
}

void LCD_mostrarConsumo(uint8_t muestra, uint16_t consumo)
{
    char linea2[20];
    
    snprintf(linea2, sizeof(linea2), "M:%u, C:%u", muestra, consumo);
    
    LCD_write(2, linea2);
}
