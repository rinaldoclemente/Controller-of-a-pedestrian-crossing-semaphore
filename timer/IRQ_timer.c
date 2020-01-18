/*********************************************************************************************************
**--------------File Info---------------------------------------------------------------------------------
** File name:           IRQ_timer.c
** Last modified Date:  2019-01-12
** Last Version:        V1.00
** Descriptions:        functions to manage T0 and T1 interrupts
** Correlated files:    timer.h
**--------------------------------------------------------------------------------------------------------
*********************************************************************************************************/
#include "lpc17xx.h"
#include "timer.h"
#include "../led/led.h"

uint16_t SinTable[45] =                                                     
{
    410, 467, 523, 576, 627, 673, 714, 749, 778,
    799, 813, 819, 817, 807, 789, 764, 732, 694, 
    650, 602, 550, 495, 438, 381, 324, 270, 217,
    169, 125, 87 , 55 , 30 , 12 , 2  , 0  , 6  ,   
    20 , 41 , 70 , 105, 146, 193, 243, 297, 353
};

/****************************************************************************** 

k=1/f'*f/n  k=f/(f'*n) k=25MHz/(f'*45)

131Hz		k=4240
147Hz		k=3779
165Hz		k=3367
175Hz		k=3175
196Hz		k=2834		
220Hz		k=2525
247Hz		k=2249
262Hz		k=2120
294Hz		k=1890		
330Hz		k=1684		
349Hz		k=1592		
392Hz		k=1417		
440Hz		k=1263		
494Hz		k=1125		
523Hz		k=1062

******************************************************************************/

int state = 0;
int flash = 0; //per vedere i secondi che passano
int blind = 0;
uint32_t flash_time = 0x00BEBC20;
uint32_t one_sec = 0x017D7840;
uint32_t five_sec = 0x07735940;
uint32_t fifteen_sec = 0x165A0BC0;
int freq = 2249;
unsigned short amplitude=10; //volume
int potentiometer = 0;

/******************************************************************************
** Function name:		Timer0_IRQHandler
**
** Descriptions:		Timer/Counter 0 interrupt handler
**
** parameters:			None
** Returned value:		None
**
******************************************************************************/

void TIMER0_IRQHandler (void)
{
		switch(state)
		{
			case 0:																				/* car = red, pedestrian = green */
				if(blind == 1)															/* case 0 for blind people */
				{
					if (flash % 2 == 0)
					{
						init_timer(1, freq);
						enable_timer(1);
					}
					else
					{
						disable_timer(1);
						reset_timer(1);
					}
					flash ++;
					if (flash >= 15)													//flash usato anche per DAC durante i 15 sec
					{
						disable_timer(1);
						reset_timer(1);
						state = 1;
						flash = 0;
						RestartTimer(flash_time, 1, 1, 0);
						LED_Off(6);
					}
					else 
						RestartTimer(0x0, 0, 0, 1);
				}
				else																				/* case 0 for normal people */
				{
					state = 1;
					flash = 0;
					RestartTimer(flash_time, 1, 1, 0);
					LED_Off(6);
				}
				break;
			
			case 1:																				/* flashing green pedestrian */
				if(blind == 1)															/* case 1 for blind people */
				{
					if (flash % 2 == 0)
					{
						LED_On(6);
						init_timer(1, freq);
						enable_timer(1);
					}
					else
					{
						LED_Off(6);
						disable_timer(1);
						reset_timer(1);
					}
					flash ++;
					if (flash >= 10)
					{
						RestartTimer(0x0, 1, 0, 0);
						flash = 0;
						state = 2;
						LED_Out(0x88);
						blind = 0;
					}
					else 
						RestartTimer(0x0, 0, 0, 1);
				}
				else																					/* case 1 for normal people */
				{
					if (flash % 2 == 0)
						LED_On(6);
					else
						LED_Off(6);
					flash ++;
					if (flash >= 10)
					{
						RestartTimer(0x0, 1, 0, 0);
						flash = 0;
						state = 2;
						LED_Out(0x88);
					}
					else 
						RestartTimer(0x0, 0, 0, 1);
				}
				break;
				
			case 3:																				/* car = yellow, pedestrian = red */
				LED_Out(0x90);			
				state = 4;
				RestartTimer(five_sec, 1, 1, 0);
				break;
			
			case 4:																				/* return to initial configuration */
				LED_Out(0x60);
				state = 0;
				if(blind == 1)
					RestartTimer(one_sec, 1, 1, 0);
				else
					RestartTimer(fifteen_sec, 1, 1, 0);				/* start again 15 seconds timer*/
				break;
			
			case 5:																				/* Maintenance state */
				if (flash % 2 == 0)
				{
					LED_Out(0x90);
					init_timer(1, freq);
					enable_timer(1);
				}
				else
				{
					LED_Out(0x00);
					disable_timer(1);
					reset_timer(1);
				}
				flash ++;
				break;
			
			default:
				break;
		}
		
  	LPC_TIM0->IR = 1;			/* clear interrupt flag */
  	return;
}


/******************************************************************************
** Function name:		Timer1_IRQHandler
**
** Descriptions:		Timer/Counter 1 interrupt handler
**
** parameters:			None
** Returned value:		None
**
******************************************************************************/
void TIMER1_IRQHandler (void)
{
		static int ticks=0;
		int value;
		//DAC management
		value = (SinTable[ticks] / 10) * amplitude; /* diviso dieci per il troppo divario tra i volumi */
		LPC_DAC->DACR = value<<6;
		
		ticks++;
		if(ticks==45) ticks=0;
  	LPC_TIM1->IR = 1;			                      /* clear interrupt flag */
  	return;
}


void PedestrianRequest(int b)
{
	if(b == 1)
		blind = 1;
	switch(state)
	{
		case 0:
			if(blind == 1){
				flash=0;
				RestartTimer(one_sec, 1, 1, 0);
			}
			else
				RestartTimer(fifteen_sec, 1, 1, 0);				/* start again 15 seconds timer*/
			break;
		
		case 1:
			state = 0;
			LED_Out(0x60);														/* return to the state of green light */
			RestartTimer(fifteen_sec, 1, 1, 0);
			break;
		
		case 2:
			state = 3;
			RestartTimer(five_sec, 0, 1, 0);					/* start 5 seconds timer to the state of the yellow light */
			break;
		
		default:
			break;
	}
}

void Maintenance(void)
{
	if(state == 0)
	{
		potentiometer = 1;
		state = 5;
		RestartTimer(one_sec, 1, 1, 0);
	}
}

void StopMaintenance(void)
{
	if(state == 5)
	{
		potentiometer = 0;
		state = 0;
		blind = 0;
		disable_timer(1);
		reset_timer(1);
		LED_Out(0x60);														/* return to the state of green light */
		RestartTimer(fifteen_sec, 1, 1, 0);
	}
}

void RestartTimer(uint32_t time, int disable, int start, int reset)
{
	if (disable == 1)
	{
		reset_timer(0);
		disable_timer(0);
	}
	
	if (reset == 1)
	{
		reset_timer(0);
		enable_timer(0);
	}
	
	if (start == 1)
	{
		init_timer(0, time);
		enable_timer(0);
	}
}

void ReadVolume(int v)
{
	amplitude = v;
}

/******************************************************************************
                              End Of File
******************************************************************************/
