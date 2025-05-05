#include "main.h"
#include "string.h"
#include "stdio.h"
#include "lcd.h"
#include "Arial12x12.h"

ARM_DRIVER_SPI *SPIDrv = &Driver_SPI1;
static GPIO_InitTypeDef GPIO_InitStruct;


char mensajeL1[100];
char mensajeL2[100];
   uint32_t positionL1;
   uint32_t positionL2;

unsigned char buffer[512];
int i;

/*FUNCIONES CONFIGURACION SPI */
void LCD_reset(){
 
  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOD_CLK_ENABLE();
  __HAL_RCC_GPIOF_CLK_ENABLE();
    
  SPIDrv -> Initialize(NULL);
  SPIDrv -> PowerControl(ARM_POWER_FULL);
  SPIDrv -> Control(ARM_SPI_MODE_MASTER | ARM_SPI_CPOL1_CPHA1 |
  ARM_SPI_MSB_LSB | ARM_SPI_DATA_BITS(8), 20000000);

  
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP; /*Modo de funcionamiento de los pines, en nuestro cado modo PUSH PULL de salida*/
  GPIO_InitStruct.Pull = GPIO_PULLUP; /*Activación de Pull-up, es decir, resistencia conectado a Vcc*/
//  GPIO_InitStruct.Speed= GPIO_SPEED_FREQ_VERY_HIGH;

  GPIO_InitStruct.Pin= PIN_RESET;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);
  HAL_GPIO_WritePin(GPIOA,PIN_RESET, GPIO_PIN_SET);

  GPIO_InitStruct.Pin=PIN_A0;
  HAL_GPIO_Init(GPIOF, &GPIO_InitStruct);
  HAL_GPIO_WritePin(GPIOF,PIN_A0, GPIO_PIN_SET);

  GPIO_InitStruct.Pin=PIN_CS;
  HAL_GPIO_Init(GPIOD, &GPIO_InitStruct);
  HAL_GPIO_WritePin(GPIOD, PIN_CS, GPIO_PIN_SET);
  
  delay(1); /*Microsegundos 1us*/
  HAL_GPIO_WritePin (GPIOA, GPIO_PIN_6 , GPIO_PIN_RESET);
  delay(1000);/*Milisegundos 1ms*/
  HAL_GPIO_WritePin (GPIOA,GPIO_PIN_6, GPIO_PIN_SET);

}/*Fin LCD_Reset*/

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
void symbolToLocalBuffer_L1(uint8_t symbol){
	uint8_t i, value1, value2;
	uint16_t offset=0;
	
	offset=25*(symbol - ' ');
	
	for (i=0; i<12; i++){
		
		value1=Arial12x12[offset+i*2+1];
		value2=Arial12x12[offset+i*2+2];
		
		buffer[i+positionL1]=value1;
		buffer[i+128+positionL1]=value2;
	}
	
	positionL1=positionL1+Arial12x12[offset];
} 

void symbolToLocalBuffer_L2(uint8_t symbol){
	uint8_t i, value1, value2;
	uint16_t offset=0;
	
	offset=25*(symbol - ' ');
	
	for (i=0; i<12; i++){
		
		value1=Arial12x12[offset+i*2+1];
		value2=Arial12x12[offset+i*2+2];
		
		buffer[i+256+positionL2]=value1;
		buffer[i+384+positionL2]=value2;
	}
	
	positionL2=positionL2+Arial12x12[offset];
} 
void symbolToLocalBuffer(uint8_t line, uint8_t symbol){
	uint8_t i, value1, value2;
	uint16_t offset=0;
	
	offset=25*(symbol - ' ');
	if(line==1){
	for (i=0; i<12; i++){
		
		value1=Arial12x12[offset+i*2+1];
		value2=Arial12x12[offset+i*2+2];

		buffer[i+positionL1]=value1;
		buffer[i+128+positionL1]=value2;
	
	}
	
	positionL1=positionL1+Arial12x12[offset];
    }
	else{
	for (i=0; i<12; i++){
		
		value1=Arial12x12[offset+i*2+1];
		value2=Arial12x12[offset+i*2+2];

		buffer[i+256+positionL2]=value1;
		buffer[i+384+positionL2]=value2;
	
	}
	
	positionL2=positionL2+Arial12x12[offset];	
	
	
	}
}

void escribirLinea(uint8_t line, char* mensaje){
int i;
int longitud = strlen(mensaje);
	for(i=0; i < longitud; i++){
		symbolToLocalBuffer(line,mensaje[i]);
	}
//LCD_update_L1();
//LCD_update_L3();
}

void limpiarLCD(void){
  for(int i= 0; i<512; i++){
    buffer[i] = 0x00;
  }
  LCD_update_L1();
  LCD_update_L2();
	LCD_update_L3();
	LCD_update_L4();
  positionL1=0;
  positionL2=0;
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