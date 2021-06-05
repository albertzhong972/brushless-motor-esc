/**
 * Albert Zhong, June 4, 2021
 * Brushless Motor ESC with BACK EMF detection
 *
 *
 * To Do:
 * Speed Control
 * Direction Control
 *
 */

#include <../Inc/stm32g071xx.h>
#include "PWM.h"

/*
 * Pin Assignments:
 *
 * PC7 - A_H
 * PA9 - B_H
 * PA8 - C_H
 *
 * PA12 - A_L
 * PA11 - B_L
 * PC1 - C_L
 *
 * PC13 - Button In
 */

//Global Variables:
int sequence_step = 0;

void GPIO_Config ( void ) {

	RCC->IOPENR |= (1 << 0); //enable Ports A, B, and C for PWM outputs
	RCC->IOPENR |= (1 << 1);
	RCC->IOPENR |= (1 << 2);

	//PC13 Button Input & PA5 LED Output
	GPIOC->MODER &= ~(3 << 26); //PC13 Input
	GPIOA->MODER &= ~(1 << 11); //PA5 Output

	//AL, BL, CL (GPIO Outputs)
	GPIOA->MODER &= ~(1 << 25); //PA12 is A_L
	GPIOA->MODER &= ~(1 << 23); //PA11 is B_L
	GPIOC->MODER &= ~(1 << 3);  //PC1 is C_L

	//AH, BH, CH (PWM outputs)
	GPIOC->MODER &= ~(1 << 14);	//alternate function on PC7 (A_H)
	GPIOC->AFR[0] |= (1 << 28);	//use AF1 on PC7 (TIM3_CH2)

	GPIOB->MODER &= ~(1 << 0);	//alternate function on PB0 (B_H)
	GPIOB->AFR[0] |= (1 << 0);	//use AF1 on PB0 (TIM3_CH3)

	GPIOA->MODER &= ~(1 << 12);	//alternate function on PA6 (C_H)
	GPIOA->AFR[0] |= (1 << 24);	//use AF1 on PA6 (TIM3_CH1)

	//Enable pull-down resisters for AH, BH, CH, AL, BL, CL
	GPIOA->PUPDR |= (1 << 25); //PA12 is A_L
	GPIOA->PUPDR |= (1 << 23); //PA11 is B_L
	GPIOC->PUPDR |= (1 << 3); //PC1 is C_L

	GPIOC->PUPDR |= (1 << 15); //PC7 (A_H)
	GPIOB->PUPDR |= (1 << 1); //PB0 (B_H)
	GPIOA->PUPDR |= (1 << 13); //PA6 (C_H)

}


//SWITCHING SEQUENCES
void AH_BL(void)
{
	GPIOA->ODR &= ~(1 << 12); 		//clear A_L (PA12)
	GPIOC->ODR &= ~(1 << 1);		//clear C_L (PC1)

	GPIOA->ODR |= (1 << 11);		//set B_L (PA11)

	TIM3->CCER &= ~(1 << 8);		//B_H PWM Output Disabled
	TIM3->CCER &= ~(1 << 0);		//C_H PWM Output Disabled

	TIM3->CCER |= (1 << 4);			//A_H PWM Output Enabled


}
void AH_CL(void)
{
	GPIOA->ODR &= ~(1 << 12); 		//clear A_L (PA12)
	GPIOA->ODR &= ~(1 << 11);		//clear B_L (PA11)

	GPIOC->ODR |= (1 << 1);			//set  C_L (PC1)

	TIM3->CCER &= ~(1 << 8);		//B_H PWM Output Disabled
	TIM3->CCER &= ~(1 << 0);		//C_H PWM Output Disabled

	TIM3->CCER |= (1 << 4);			//A_H PWM Output Enabled
}
void BH_CL(void)
{
	GPIOA->ODR &= ~(1 << 12); 		//clear A_L (PA12)
	GPIOA->ODR &= ~(1 << 11);		//clear B_L (PA11)

	GPIOC->ODR |= (1 << 1);			//set C_L (PC1)

	TIM3->CCER &= ~(1 << 4);		//A_H PWM Output Disabled
	TIM3->CCER &= ~(1 << 0);		//C_H PWM Output Disabled

	TIM3->CCER |= (1 << 8);			//B_H PWM Output Enabled
}
void BH_AL(void)
{
	GPIOA->ODR &= ~(1 << 11);		//clear B_L (PA11)
	GPIOC->ODR &= ~(1 << 1);		//clear C_L (PC1)

	GPIOA->ODR |= (1 << 12); 		//set A_L (PA12)

	TIM3->CCER &= ~(1 << 4);		//A_H PWM Output Disabled
	TIM3->CCER &= ~(1 << 0);		//C_H PWM Output Disabled

	TIM3->CCER |= (1 << 8);			//B_H PWM Output Enabled

}
void CH_AL(void)
{
	GPIOA->ODR &= ~(1 << 11);		//clear B_L (PA11)
	GPIOC->ODR &= ~(1 << 1);		//clear C_L (PC1)

	GPIOA->ODR |= (1 << 12); 		//set A_L (PA12)

	TIM3->CCER &= ~(1 << 4);		//A_H PWM Output Disabled
	TIM3->CCER &= ~(1 << 8);		//B_H PWM Output Disabled

	TIM3->CCER |= (1 << 0);			//C_H PWM Output Enabled

}
void CH_BL(void)
{
	GPIOA->ODR &= ~(1 << 12); 		//clear A_L (PA12)
	GPIOC->ODR &= ~(1 << 1);		//clear C_L (PC1)

	GPIOA->ODR |= (1 << 11);		//set B_L (PA11)

	TIM3->CCER &= ~(1 << 4);		//A_H PWM Output Disabled
	TIM3->CCER &= ~(1 << 8);		//B_H PWM Output Disabled

	TIM3->CCER |= (1 << 0);			//C_H PWM Output Enabled

}


//BACK-EMF detection

void BEMF_A_RISING(void) {

	COMP1->CSR &= ~(COMP_CSR_EN);					//Disable COMP1

	COMP1->CSR &= ~(COMP_CSR_INMSEL_Msk);
	COMP1->CSR |= (8 << 4); 						//Set PA0 (ZC_A) as the negative input of comparator 1

	EXTI->FTSR1 &= ~(EXTI_FTSR1_FT17);				//Disable falling edge interrupt trigger
	EXTI->RTSR1 |= EXTI_RTSR1_RT17;					//Enable rising edge interrupt trigger

	COMP1->CSR |= COMP_CSR_EN;						//Enable COMP1

}

void BEMF_A_FALLING(void) {

	COMP1->CSR &= ~(COMP_CSR_EN);					//Disable COMP1

	COMP1->CSR &= ~(COMP_CSR_INMSEL_Msk);
	COMP1->CSR |= (8 << 4); 						//Set PA0 (ZC_A) as the negative input of comparator 1

	EXTI->RTSR1 &= ~(EXTI_RTSR1_RT17);				//Disable rising edge interrupt trigger
	EXTI->FTSR1 |= EXTI_FTSR1_FT17;					//Enable falling edge interrupt trigger

	COMP1->CSR |= COMP_CSR_EN;						//Enable COMP1

}

void BEMF_B_RISING(void) {

	COMP1->CSR &= ~(COMP_CSR_EN);					//Disable COMP1

	COMP1->CSR &= ~(COMP_CSR_INMSEL_Msk);
	COMP1->CSR |= (6 << 4); 						//Set PB1 (ZC_B) as the negative input of comparator 1

	EXTI->FTSR1 &= ~(EXTI_FTSR1_FT17);				//Disable falling edge interrupt trigger
	EXTI->RTSR1 |= EXTI_RTSR1_RT17;					//Enable rising edge interrupt trigger

	COMP1->CSR |= COMP_CSR_EN;						//Enable COMP1

}

void BEMF_B_FALLING(void) {

	COMP1->CSR &= ~(COMP_CSR_EN);					//Disable COMP1

	COMP1->CSR &= ~(COMP_CSR_INMSEL_Msk);
	COMP1->CSR |= (6 << 4); 						//Set PB1 (ZC_B) as the negative input of comparator 1

	EXTI->RTSR1 &= ~(EXTI_RTSR1_RT17);				//Disable rising edge interrupt trigger
	EXTI->FTSR1 |= EXTI_FTSR1_FT17;					//Enable falling edge interrupt trigger

	COMP1->CSR |= COMP_CSR_EN;						//Enable COMP1

}

void BEMF_C_RISING(void) {

	COMP1->CSR &= ~(COMP_CSR_EN);					//Disable COMP1

	COMP1->CSR &= ~(COMP_CSR_INMSEL_Msk);
	COMP1->CSR |= (7 << 4); 						//Set PC4 (ZC_C) as the negative input of comparator 1

	EXTI->FTSR1 &= ~(EXTI_FTSR1_FT17);				//Disable falling edge interrupt trigger
	EXTI->RTSR1 |= EXTI_RTSR1_RT17;					//Enable rising edge interrupt trigger

	COMP1->CSR |= COMP_CSR_EN;						//Enable COMP1

}

void BEMF_C_FALLING(void) {

	COMP1->CSR &= ~(COMP_CSR_EN);					//Disable COMP1

	COMP1->CSR &= ~(COMP_CSR_INMSEL_Msk);
	COMP1->CSR |= (7 << 4); 						//Set PC4 (ZC_C) as the negative input of comparator 1

	EXTI->RTSR1 &= ~(EXTI_RTSR1_RT17);				//Disable rising edge interrupt trigger
	EXTI->FTSR1 |= EXTI_FTSR1_FT17;					//Enable falling edge interrupt trigger

	COMP1->CSR |= COMP_CSR_EN;						//Enable COMP1

}


void set_next_step(void) {
	switch (sequence_step) {
	case 0:
		AH_BL();
		BEMF_C_RISING();
		break;
	case 1:
		AH_CL();
		BEMF_B_FALLING();
		break;
	case 2:
		BH_CL();
		BEMF_A_RISING();
		break;
	case 3:
		BH_AL();
		BEMF_C_FALLING();
		break;
	case 4:
		CH_AL();
		BEMF_B_RISING();
		break;
	case 5:
		CH_BL();
		BEMF_A_FALLING();
		break;
	}
}


void ADC_COMP_IRQHandler(void)
{
	NVIC_DisableIRQ(ADC1_COMP_IRQn); // need this to prevent continuous motor spin when button released
	EXTI->FPR1 |= (1 << 17); //write to clear interrupt request
	EXTI->RPR1 |= (1 << 17); //write to clear interrupt request

	//Make sure that zero-crossing is actually happening
	for (int i = 0; i < 10; i++)
	{
		if(sequence_step & 1) //if sequence = 1, 3, 5
		{
			if(!(COMP1->CSR & COMP_CSR_VALUE))
				i -= 1;
		}
		else
		{
			if(COMP1->CSR & COMP_CSR_VALUE)
				i -= 1;
		}
	}

	set_next_step();
	sequence_step++;
	sequence_step %= 6;

}


int switch_delay = 3500;

int main(void) {

	GPIO_Config();
	PWM_Config();
	Timer_Config();

	RCC->APBENR2 |= RCC_APBENR2_SYSCFGEN; //Enable clock for COMP
	//COMP1->CSR |= (1 << 16); //COMP1 Hysteresis Mode = LOW ( in case of noisy signals)
	COMP1->CSR |= (2 << 8); //Set PA1 as positive input for Comparator 1
	EXTI->IMR1 |= EXTI_IMR1_IM17; //Setup EXTI17 line as an interrupt (COMP1 output)

	NVIC_SetPriority(ADC1_COMP_IRQn, 0x03);


	while(1)
	{
		if(!(GPIOC->IDR & (1 << 13)))
		{

			//Start of motor rotating, slowly ramp up to a constant speed
			while(switch_delay > 1000)
			{
				Delay_us(switch_delay);
				set_next_step();
				sequence_step++;
				sequence_step %=6;
				switch_delay -= 5;
			}
			//From here, sequence switching will be from back-EMF zero-crossing interrupts from the comparator
			NVIC_EnableIRQ(ADC1_COMP_IRQn);	//Enable analog comparator interrupt
			//COMP1->CSR |= COMP_CSR_EN;				//Enable COMP1


		}
		else
		{
			//COMP1->CSR &= ~COMP_CSR_EN;				//Disable COMP1
			NVIC_DisableIRQ(ADC1_COMP_IRQn);//disable analog comparator interrupt

			TIM3->CCER &= ~(1 << 4);	//A_H PWM Output Disabled
			TIM3->CCER &= ~(1 << 8);	//B_H PWM Output Disabled
			TIM3->CCER &= ~(1 << 0);	//C_H PWM Output Disabled

			GPIOA->ODR &= ~(1 << 12); 		//clear A_L (PA12)
			GPIOA->ODR &= ~(1 << 11);		//clear B_L (PA11)
			GPIOC->ODR &= ~(1 << 1);		//clear C_L (PC1)

			switch_delay = 3500; //reset switch delay


		}
	}
}
