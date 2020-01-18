/*********************************************************************************************************
**--------------File Info---------------------------------------------------------------------------------
** File name:           IRQ_adc.c
** Last modified Date:  2019-01-12
** Last Version:        V1.00
** Descriptions:        functions to manage A/D interrupts
** Correlated files:    adc.h
**--------------------------------------------------------------------------------------------------------       
*********************************************************************************************************/

#include "lpc17xx.h"
#include "adc.h"
#include "../led/led.h"
#include "../timer/timer.h"

/*----------------------------------------------------------------------------
  A/D IRQ: Executed when A/D Conversion is ready (signal from ADC peripheral)
 *----------------------------------------------------------------------------*/

unsigned short AD_current;   
unsigned short AD_last = 0xFF;     /* Last converted value               */ 
extern int potentiometer;

void ADC_IRQHandler(void) {
  	
  AD_current = ((LPC_ADC->ADGDR>>4) & 0xFFF);   /* Read Conversion Result  //valore del potenziometro */
  if(AD_current != AD_last){
		if(potentiometer==1)	                      //solo se siamo in maintenance
		ReadVolume((0xFFF - AD_current)/200);       //potenziometro al contrario 
																					      //diviso 200 per creare 20 sottointervalli 
																					      //di volume perchè inizialmente sono 4096
		
		AD_last = AD_current;
  }
	
}
