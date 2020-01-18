/*********************************************************************************************************
**--------------File Info---------------------------------------------------------------------------------
** File name:           IRQ_RIT.c
** Last modified Date:  2019-01-12
** Last Version:        V1.00
** Descriptions:        functions to manage T0 and T1 interrupts
** Correlated files:    RIT.h
**--------------------------------------------------------------------------------------------------------
*********************************************************************************************************/
#include "lpc17xx.h"
#include "RIT.h"
#include "../led/led.h"
#include "../timer/timer.h"

/******************************************************************************
** Function name:		RIT_IRQHandler
**
** Descriptions:		REPETITIVE INTERRUPT TIMER handler
**
** parameters:			None
** Returned value:		None
**
******************************************************************************/

int down_i0 = 0;
int down_k1 = 0;
int down_k2 = 0;
extern int freq;

void RIT_IRQHandler (void)
{					
	static int right=0;
	static int left=0;	
	
	/* button management */
	
	if(down_i0!=0){ 
		if((LPC_GPIO2->FIOPIN & (1<<10)) == 0){	/* INT0 pressed */
			down_i0++;
			init_timer(1, freq);
			enable_timer(1);
			PedestrianRequest(1);
		}
		else {	/* button released */
			down_i0=0;
			disable_timer(1);
			reset_timer(1);
			NVIC_EnableIRQ(EINT0_IRQn);							 /* enable Button interrupts			*/
			LPC_PINCON->PINSEL4    |= (1 << 20);     /* External interrupt 0 pin selection */
		}
	}
	
	if(down_k1!=0){ 
		if((LPC_GPIO2->FIOPIN & (1<<11)) == 0){	/* KEY1 pressed */
			down_k1++;				
			PedestrianRequest(0);
		}
		else {	/* button released */
			down_k1=0;			
			NVIC_EnableIRQ(EINT1_IRQn);							 /* enable Button interrupts			*/
			LPC_PINCON->PINSEL4    |= (1 << 22);     /* External interrupt 0 pin selection */
		}
	}
	
	if(down_k2!=0){ 
		if((LPC_GPIO2->FIOPIN & (1<<12)) == 0){	/* KEY2 pressed */
			down_k2++;				
			PedestrianRequest(0);
		}
		else {	/* button released */
			down_k2=0;
			NVIC_EnableIRQ(EINT2_IRQn);							 /* enable Button interrupts			*/
			LPC_PINCON->PINSEL4    |= (1 << 24);     /* External interrupt 0 pin selection */
		}
	}
		
	/* joystick management */
	if((LPC_GPIO1->FIOPIN & (1<<28)) == 0){	/* Joytick Right pressed */
		right++;
		switch (right)
		{
			case 1:
				Maintenance();	
				break;
			default:
				break;
		}
	}
	else
		right=0;
	
	if((LPC_GPIO1->FIOPIN & (1<<27)) == 0){	/* Joytick Left pressed */
		left++;
		switch (left)
		{
			case 1:
				StopMaintenance();	
				break;
			default:
				break;
		}
	}
	else
		left=0;
		
	/* ADC management */
	ADC_start_conversion();		
			
  LPC_RIT->RICTRL |= 0x1;	/* clear interrupt flag */
}

/******************************************************************************
**                            End Of File
******************************************************************************/
