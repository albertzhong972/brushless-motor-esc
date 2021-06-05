/*
 * PWM.c
 *
 *  Created on: May 2, 2021
 *      Author: jongb
 */

#include "PWM.h"
//configure TIM3 for 20ms PWM
void PWM_Config(void)
{

	RCC->APBENR1 |= (1 << 1);	//enable clock for TIM3
	TIM3->PSC = 0;	//Prescaler Value
	TIM3->ARR = 510;	//Reload Value (PWM period = reload value / timer clock frequency). Default clock frequency = 16MHz

	TIM3->CCR1 = 60;	//Adjust Duty Cycle
	TIM3->CCMR1 |= (6 << 4);	//PWM mode 1 for TIM3_CH1
	TIM3->CCER |= (1 << 0);	//Capture/Compare 1 Output Enabled

	TIM3->CCR2 = 60;	//Adjust Duty Cycle
	TIM3->CCMR1 |= (6 << 12);	//PWM mode 1 for TIM3_CH2
	TIM3->CCER |= (1 << 4);	//Capture/Compare 2 Output Enabled

	TIM3->CCR3 = 60;	//Adjust Duty Cycle
	TIM3->CCMR2 |= (6 << 4);	//PWM mode 1 for TIM3_CH3
	TIM3->CCER |= (1 << 8);	//Capture/Compare 3 Output Enabled

	TIM3->CR1 |= TIM_CR1_CEN;	//enable counter for TIM3
	while (!(TIM3->SR & (1<<0)));

}

void Timer_Config(void)
{
	RCC->APBENR1 |= (1 << 0);	//enable clock for TIM2

	TIM2->PSC = 16-1;	//Prescaler Value , timer bus runs at 16 MHz, 16Mhz / 16 = 1 us
	TIM2->ARR = 0xffff;	//Reload Value

	TIM2->CR1 |= TIM_CR1_CEN;	//enable counter for TIM2
	while (!(TIM2->SR & (1<<0)));
}

void Delay_us (uint16_t us)
{
	/************** STEPS TO FOLLOW *****************
	1. RESET the Counter
	2. Wait for the Counter to reach the entered value. As each count will take 1 us,
		 the total waiting time will be the required us delay
	************************************************/
	TIM2->CNT = 0;
	while (TIM2->CNT < us);


}

void Delay_ms (uint16_t ms)
{
	for (uint16_t i=0; i<ms; i++)
	{
		Delay_us (1000); // delay of 1 ms
	}
}


