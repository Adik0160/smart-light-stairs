/*
 * master.cpp
 * ATmega32 16MHz
 * Created: 2016-07-09 00:37:01
 * Author : Adrian Katulski
 */ 



#include <avr/io.h>
#include <util/delay.h>
#include <avr/interrupt.h>
#include <stdio.h>
#include <stdlib.h>

#include "defines.h"
#include "HD44780.h"
#include "sonar.h"

//

///////---ZMIENNE GLOBALNE UWAGA!---////////////
volatile uint8_t runningSonar_Global = 'F'; //F - false - trwanie jakiegoœ pomiaru
volatile uint8_t echoRising_Global = false;
//volatile uint8_t configRef_Global = false; //przy konfiguracji czujnika musi byæ true
volatile uint32_t timerCounter_Global = 0;
volatile uint8_t actualLedPin_Global = 1;
volatile uint8_t positionLedF_Global = 0;
////////////////////////////////////////////////

Sonar hcsr04_0(0,0);
Sonar hcsr04_1(1,0);
/*

void sonar_config(void)
{
	char temp[16];
	LCD_LED_ON();
	_delay_ms(DELAY_BTW_TESTS_MS);
	while(1)
	{
	hcsr04_0.measure(true); //sonarnumber = 0, configref = true
	while(runningSonar_Global != 'F') asm("NOP"::); //czekamy na skonczenie siê pomiaru
	if(hcsr04_0.actualTime != 0) break;
	_delay_ms(DELAY_BTW_TESTS_MS);
	}
	
	sprintf( temp, "CZUJNIK 1: %d", hcsr04_0.refTime) ;
	LCD_WriteText(temp);
	
	_delay_ms(DELAY_BTW_TESTS_MS);
	while(1)
	{
	hcsr04_1.measure(true); //sonarnumber = 1, configref = true
	while(runningSonar_Global != 'F') asm("NOP"::); //czekamy na skonczenie siê pomiaru
	if(hcsr04_1.actualTime != 0) break;
	_delay_ms(DELAY_BTW_TESTS_MS);
	}
	LCD_GoTo(0,1);	
	sprintf( temp, "CZUJNIK 2: %d", hcsr04_1.refTime) ;
	LCD_WriteText(temp);
	_delay_ms(4000);
	LCD_Clear();
	
}

*/



void sonars_config(void)
{
	char temp[16];
	LCD_LED_ON();
	LCD_Home();
	LCD_GoTo(1,0);
	LCD_WriteText((char*)"CZUJNIK NR. 1");
	sprintf(temp ,"Odl REF: %dcm" ,hcsr04_0.avarage_config());
	LCD_GoTo(1,1);
	LCD_WriteText(temp);
	_delay_ms(1500);
	
	LCD_Clear();
	LCD_GoTo(1,0);
	LCD_WriteText((char*)"CZUJNIK NR. 2");
	sprintf(temp ,"Odl REF: %dcm" ,hcsr04_1.avarage_config());
	LCD_GoTo(1,1);
	LCD_WriteText(temp);
	
	_delay_ms(1500);
	LCD_LED_OFF();
	LCD_Clear();	

} 

void port_init(void) //inicjalizacja portów 
{
	DDRA &=~ (1<<BUTTON_1)|(1<<BUTTON_2)|(1<<BUTTON_3)|(1<<BUTTON_4); //input
	PORTA |= (1<<BUTTON_1)|(1<<BUTTON_2)|(1<<BUTTON_3)|(1<<BUTTON_4); //vcc pull up
	
	DDRB |= 0xff; //output
	PORTB = 0; 
	
	DDRD |= (1<<TRIGGER_1)|(1<<TRIGGER_0)|(1<<LCD_LED); //output
	DDRD &=~ (1<<ECHO_1)|(1<<ECHO_0); //input
	
	LCD_Initalize();
}

void timer_init(void) 
{
	TIMSK |= (1<<TOIE0); //timer 0 maska przerwania w³¹czona
	TIMSK |= (1<<OCIE1A)|(1<<OCIE1B)|(1<<TOIE1); //timer 1 maska przerwania w³aczona na ocie1a ocie1hb oraz przez przepe³nienie
	
	
	ICR1 = 16000;
	TCCR1A |= (1<<WGM11); //TIMER 1 MODE 14
	TCCR1B |= (1<<WGM13)|(1<<WGM12); //PRESKALLER 1 1kHz
}

void ex_interrupt_init(void)
{
	MCUCR |= (1<<ISC10)|(1<<ISC00); //wywo³ywanie przerwania przy kia¿dym zboczu - zewnêtrzne przerwanie int0
	GICR |= (1<<INT1)|(1<<INT0);	//w³aczenie przerwania int0s
}


void start_lighting_init(void)
{
	OCR1A = 0;
	OCR1B = 13000;
	actualLedPin_Global = 1;
	positionLedF_Global = 1;
	TIMSK |= (1<<OCIE1A);
	TCCR1B |= (1<<CS10); //pod³aczenie zegara pod licznik
}




int main(void)
{

	timer_init();
	_delay_ms(100);
	port_init();
	ex_interrupt_init();	
	sei();
	sonars_config();
	start_lighting_init();

    for (;;) 
    {	
			
			
		
			
		
			
/*		sprintf(temp,"-------%d--------",hcsr04_0.measure(false,true));
		LCD_GoTo(0,0);
		LCD_WriteText(temp);
		_delay_ms(DELAY_BTW_TESTS_MS);
*/			
		/*
		if (!(PINA & (1<<BUTTON_1)))
		{	
			_delay_ms(20);	
			while(!(PINA & (1<<BUTTON_1))) asm("NOP"::);
			do
			{
			hcsr04_0.measure(true);
			PORTB |= 0xff;
			while(runningSonar_Global != 'F') asm("NOP"::);
			PORTB = 0;
			}while(hcsr04_0.actualTime == -1);
			sprintf(temp,"Czujnik 0: %dcm", hcsr04_0.refTime);
			LCD_Home();
			LCD_Clear();
			LCD_WriteText(temp);
		}
		_delay_ms(100);
 		if (hcsr04_0.comparate())
 		{
 			
			 if(hcsr04_0.actualTime != -1)	PORTB = 0xff;
 		} 
 		else
 		{
			PORTB = 0;
 		}
		sprintf(temp,"Aktualna: %dcm", hcsr04_0.actualTime);
		LCD_GoTo(0,1);
		LCD_WriteText(temp);
		*/		
	}
	
}

ISR(INT1_vect) //CZUJNIK NR 1
{
	if (runningSonar_Global == 1)
	{
		if(echoRising_Global){
			TCCR0 |= 1<<CS00; //startujemy licznik
			TCNT0 = 0; //zerowanie licznika timera0
			timerCounter_Global = 0; //zerowanie licznika mikrosekund
			echoRising_Global  = 0; //czekamy na nastêpny stan - to bêdzie opadajacy 			
		}else {
			TCCR0 &=~ (1<<CS00); //od³¹czamy zegar od timera 0
			if(((timerCounter_Global * 16) / 58) < 300)
			{
				hcsr04_1.actualTime = (int)((timerCounter_Global * 16) / 58);
	/*			if(configRef_Global) {
					hcsr04_1.refTime = hcsr04_1.actualTime - 2;
					configRef_Global = false;
				}	*/
			} else {
				hcsr04_1.actualTime = -1; //przeslij b³¹d
			}
			runningSonar_Global = 'F';		
		}
	}
}

ISR(INT0_vect) //CZUJNIK NR 0
{
	if (runningSonar_Global == 0)
	{
		if(echoRising_Global){
			TCCR0 |= 1<<CS00; //startujemy licznik
			TCNT0 = 0; //zerowanie licznika timera0
			timerCounter_Global = 0; //zerowanie licznika mikrosekund
			echoRising_Global  = 0; //czekamy na nastêpny stan - to bêdzie opadajacy 			
		}else {
			TCCR0 &=~ (1<<CS00); //od³¹czamy zegar od timera 0
			if(((timerCounter_Global * 16) / 58) < 300)
			{
				hcsr04_0.actualTime = (int)((timerCounter_Global * 16) / 58);				
/*				if(configRef_Global) {
					hcsr04_0.refTime = hcsr04_0.actualTime - 2;
					configRef_Global = false;
				}	*/
			} else {
				hcsr04_0.actualTime = -1;				
			}//else ustawiaj rezultat na -1czyli wywalaj b³¹d
			runningSonar_Global = 'F';		
		}
	}
}

ISR(TIMER0_OVF_vect) //licznik czasu impulsów z czujnika 0 lub 1
{
	timerCounter_Global++;
/* 	if (((timerCounter_Global * 16) / 58) > 400)
 	{
 		runningSonar_Global = 'F';
 		TCCR0 &=~ (1<<CS00);
 	}*/
}
/*
 		if(echoRising_Global)
 		{
 			TCCR0 |= 1<<CS00;
 			TCNT0 = 0;
 			timerCounter_Global = 0;
 			echoRising_Global = 0;
 		} else {
			TCCR0 &=~ 1<<CS00;
 			echoRising_Global = 1;
 			if(((timerCounter_Global * 16) / 58) < 278) hc04.actual_time = (timerCounter_Global * 16) / 58;
 			runningSonar_Global = false;
 		}
*/

ISR(TIMER1_COMPA_vect)
{
	static volatile uint8_t temp = 1;
	if(positionLedF_Global == 1)
	{
		PORTB &= ~(temp);
		if(OCR1A < OCR1B)
		{
			OCR1A += 70;
		} else{
			temp = temp<<1;
			actualLedPin_Global |= temp;
			if(actualLedPin_Global == 0xff && temp == 0)
			{
				positionLedF_Global = 2;
				temp = 1;
				TIMSK &=~ (1<<OCIE1A); //WY£¥CZA PRZERWANIE KIEDY
			}
			OCR1A = 0;
		}
	}		
}
ISR(TIMER1_COMPB_vect)
{	
	static volatile uint16_t delayLedMS = 0;
 	switch(positionLedF_Global)
	{
		case 2:
		if (delayLedMS > 1000)
		{
			positionLedF_Global = 3;
		} else {
			delayLedMS++;
		}
		case 1:
		PORTB &= ~actualLedPin_Global;
		break;
		
		case 3:
		if (OCR1B == 0)
		{
			positionLedF_Global = 0;
			hcsr04_0.compareFlag = 0;
		} else {
			OCR1B -= 10;
		}
		PORTB &= ~actualLedPin_Global;
		break;
	}
}
 ISR(TIMER1_OVF_vect)
{
	if(positionLedF_Global != 0)
	PORTB = actualLedPin_Global;
}



/////////////////////////////////INT0 - CZUJNIK NR 0
/////////////////////////////////INT1 - CZUJNIK NR 1

