#include "cmsis_os2.h"                          // CMSIS RTOS header file
#include "sensorDistancia.h"
#include "rtc.h"

/*----------------------------------------------------------------------------
 *     										 Thread SENSOR
 *---------------------------------------------------------------------------*/
 //AFORO
uint16_t rango;
uint16_t distancia = 0;
uint8_t cnt_plazas = PLAZAS_MAX;
void control_aforo(int d);

void control_aforo(int d) {
	
  if (d <= DIST_MIN ){
	 if(cnt_plazas >0){
    cnt_plazas--; //Esta entrando un coche 	   
    rellenarLCD_L1();	 
		printf("Entrada: %d\n", cnt_plazas);
  }
} 
	
  if (d > DIST_MED1 && d <= DIST_MED2) {  //El coche esta saliendo 
    cnt_plazas++;
	  // coger_hora();
		rellenarLCD_L2();		
  }
	if (d > DIST_MED2 ) {  //El coche esta saliendo 		        
			 rellenarLCD_L3();			 
 }	
}
