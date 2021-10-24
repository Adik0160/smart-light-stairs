/*
 * master.cpp
 * ATmega88PA 16MHz
 * Created: 2016-07-09 00:37:01
 */ 



#include <avr/io.h>
#include <util/delay.h>
#include <avr/interrupt.h>
#include <stdio.h>
#include <stdlib.h>
#include <util/twi.h>

#include "defines.h"
#include "sonar.h"
#include "i2c_slave.h"

//

///////---ZMIENNE GLOBALNE UWAGA!---///////////////////////////////////////////////////////////////////////
volatile uint8_t runningSonar_Global = 'F'; //F - false - trwanie jakiegoœ pomiaru
volatile uint8_t echoRising_Global = false;
//volatile uint8_t configRef_Global = false; //przy konfiguracji czujnika musi byæ true
volatile uint32_t timerCounter_Global = 0;
volatile uint8_t actualLedPin_Global = 0;
volatile uint8_t positionLedF_Global = 0; //stage
volatile uint16_t delayLedMS_Global = 0;

volatile uint8_t tempLeft_Global = 0;
volatile uint8_t tempRight_Global = 0; //hack 2

volatile uint8_t i2cBuff_Global = 0;

volatile uint8_t relayTime_Global = 5; // [min]
volatile uint8_t ledTimeOn_Global = 8; // [sec]
volatile uint8_t ledIncrease = 30;//0   [ms]
class ledBrightConfig{//TODO: ogarnaæ t¹ klase i to równaniedo czujnika œwiat³a    (valueMax - valueMin) * actualFromADC
public:										//									--------------------------------------  + valueMin = ledBright
	volatile uint8_t ledBright;													//					255(albo 256 nie wiem)
	volatile uint8_t	valueMax;
	volatile uint8_t valueMin;
	volatile uint8_t actualFromADC;
	ledBrightConfig(uint8_t lb, uint8_t vma, uint8_t vmin, uint8_t afa)
	{
		ledBright  = lb;
		valueMax = vma;
		valueMin = vmin;
		actualFromADC = afa;
	}
	void ledBrighGenerate(void)
	{
		actualFromADC = ADCH;
		ledBright = ((valueMax - valueMin) * actualFromADC / 255) + valueMin;
	}
	
};

ledBrightConfig ledBrightConfig(100, 100, 100, 255); // 100% œwiecenia, vmax 100%, vmin 0%

//////////////////////////////////////////////////////////////////////////////////////////////////////////////

enum position {up, down, both};

Sonar hcsr04_2(2,0);
Sonar hcsr04_3(3,0);


class i2cBufStatus{
	public:
	volatile uint8_t ready;
	volatile uint8_t slaveTranciv;
	volatile char bufSlaveReciv[10];
	volatile char bufSlaveTranciv[10];
	i2cBufStatus(uint8_t r, uint8_t s)
	{
		ready = r;
		slaveTranciv = s;
	}
};
i2cBufStatus i2cBufStatus(0,0);

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
/*
void ADC_config(void)
{
	ADMUX |= (1<<REFS1)|(1<<REFS0)|(1<<ADLAR);
	ADCSRA |= (1<<ADSC)|(1<<ADPS0)|(1<<ADPS1)|(1<<ADPS2)|(1<<ADEN);
}


void ADC_start_converting(void)
{
	ADCSRA |= (1<<ADSC);
	while(ADCSRA & (1<<ADSC)) asm("NOP"::);
	ledBrightConfig.ledBrighGenerate();	
}
*/

void sonars_config(void)
{
//	char temp[16];
//	LCD_LED_ON();
//	LCD_Home();
//	LCD_GoTo(1,0);
//	LCD_WriteText((char*)"CZUJNIK NR. 1");
//	sprintf(temp ,"Odl REF: %dcm" ,hcsr04_2.avarage_config());
//	LCD_GoTo(1,1);
//	LCD_WriteText(temp);
//	_delay_ms(1500);
	_delay_ms(300);
	hcsr04_2.avarage_config();
	_delay_ms(300);
	hcsr04_3.avarage_config();
	
//	LCD_Clear();
//	LCD_GoTo(1,0);
//	LCD_WriteText((char*)"CZUJNIK NR. 2");
//	sprintf(temp ,"Odl REF: %dcm" ,hcsr04_3.avarage_config());
//	LCD_GoTo(1,1);
//	LCD_WriteText(temp);
	
//	_delay_ms(1500);
//	LCD_LED_OFF();
//	LCD_Clear();	

} 

void port_init(void) //inicjalizacja portów 
{

	DDRD |= 0xff; //output
	PORTD = 0; 
	
	DDRB |= (1<<TRIGGER_3)|(1<<TRIGGER_2); //output
	DDRB &=~ (1<<ECHO_3)|(1<<ECHO_2); //input
	
	DDRC |= (1<<Relay);

}

void timer_init(void) //TODO: OGARNAÆ DWA TIMERY
{
	TIMSK0 |= (1<<TOIE0); //timer 0 maska przerwania w³¹czona
	TIMSK1 |= (1<<OCIE1A)|(1<<OCIE1B)|(1<<TOIE1); //timer 1 maska przerwania w³aczona na ocie1a ocie1hb oraz przez przepe³nienie
	
	
	ICR1 = 16001;
	TCCR1A |= (1<<WGM11); //TIMER 1 MODE 14
	TCCR1B |= (1<<WGM13)|(1<<WGM12); //PRESKALLER 1 1kHz
}

void ex_interrupt_init(void)
{
	PCICR = (1<<PCIE0); //w³¹czenie flagi przerwania
	PCMSK0 |= (1<<PCINT0)|(1<<PCINT1); //dwa piny przerwania bedo dzia³aæ
}


void start_lighting(position goradol)
{
	
	if(tempRight_Global == 0 && tempLeft_Global == 0 && goradol == both) {
		tempLeft_Global = 1;
		tempRight_Global = (1<<7);
		actualLedPin_Global |= 1|(1<<7);
	}
	
	if(tempLeft_Global == 0 && goradol == down) {
		tempLeft_Global = 1;
		actualLedPin_Global |= 1;
	}
	if(tempRight_Global == 0 && goradol == up)	{
		tempRight_Global = (1<<7);
		actualLedPin_Global |= (1<<7);
	}
	
	if (positionLedF_Global == 0)
	{
		OCR1A = 0;
		OCR1B = (16000 * ledBrightConfig.ledBright)/100;
		positionLedF_Global = 1;
			
		TIMSK1 |= (1<<OCIE1A);
		TCCR1B |= (1<<CS10); //pod³aczenie zegara pod licznik
	}
	
	if (positionLedF_Global == 2)
	{
		delayLedMS_Global = 0;
	}
	
	if (positionLedF_Global == 3)
	{
		positionLedF_Global = 2;
		delayLedMS_Global = 0;
		OCR1B = (16000 * ledBrightConfig.ledBright)/100;
	}
	
}


void i2cConfigTranciv(void)
{
	if (i2cBufStatus.bufSlaveReciv[0] == '#')
	{
		i2cBufStatus.bufSlaveTranciv[0] = hcsr04_2.refTime;
		i2cBufStatus.bufSlaveTranciv[1] = hcsr04_3.refTime;
	}
	
	if (i2cBufStatus.bufSlaveReciv[0] == 'C')
	{
		relayTime_Global = i2cBufStatus.bufSlaveReciv[1];
		ledTimeOn_Global = i2cBufStatus.bufSlaveReciv[2];
		ledIncrease = i2cBufStatus.bufSlaveReciv[3];
		ledBrightConfig.valueMax = i2cBufStatus.bufSlaveReciv[4];
		ledBrightConfig.valueMin = i2cBufStatus.bufSlaveReciv[5];	
	}
	
	if (i2cBufStatus.bufSlaveReciv[0] == 'F')
	{
		i2cBufStatus.bufSlaveTranciv[0] = ledBrightConfig.actualFromADC;
	}

}



int main(void)
{
	uint8_t relayTimeCompare = 0;
	timer_init();
	_delay_ms(100);
	port_init();
	i2c_init();
	ex_interrupt_init();
//	ADC_config();
	sei();
	sonars_config();
	
    for (;;) 
    {	
/*			
			switch (positionLedF_Global)
			{
				case 0:
				LCD_Clear();
				LCD_WriteText((char*)"0");
				break;
				
				case 1:
				LCD_Clear();
				LCD_WriteText((char*)"1");
				break;
				
				case 2:
				LCD_Clear();
				LCD_WriteText((char*)"2");
				break;
				
				case 3:
				LCD_Clear();
				LCD_WriteText((char*)"3");
				break;
			}
			
*/			

			for (uint16_t y = 0; y < 300; y++) // 
			{
				if(i2cBufStatus.ready == 1 && i2cBufStatus.bufSlaveReciv[0] == '!')
				{
					if (positionLedF_Global == 0) /*ADC_start_converting()*/;
					i2cBufStatus.ready = 0;
					i2cBufStatus.bufSlaveReciv[0] = 0;
				}
				_delay_ms(1);
			}
					

			if (relayTimeCompare >= relayTime_Global*200) RELAY_OFF(); else relayTimeCompare++;

 			if (hcsr04_2.comparate(true) && hcsr04_2.actualTime != -1 && i2cBuff_Global == 0)
 			{
				RELAY_ON();
 				start_lighting(up);	
				relayTimeCompare = 0;	
 			}
 			if (hcsr04_3.comparate(true) && hcsr04_3.actualTime != -1 && i2cBuff_Global == 0)
 			{
				RELAY_ON();
 				start_lighting(down);
				relayTimeCompare = 0;
 			}
			
			if(i2cBufStatus.ready == 1 && i2cBufStatus.bufSlaveReciv[0] == '3') //TODO: nie dzia³a
			{
				sonars_config();
				i2cBufStatus.ready = 0;
				i2cBufStatus.bufSlaveReciv[0] = 0;
			}
			
			
			
			
			
		
			
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

ISR(PCINT0_vect) //TODO: PRZERWANIE ZEWNÊTRZNE CZUJNIKA DRUGIEGO I TRZECIEGO, SPRAWDZIÆ CZY WGL DZIA£A
{
	if (runningSonar_Global == 2 || runningSonar_Global == 3)
	{
		if(echoRising_Global){
			TCCR0B |= 1<<CS00; //startujemy licznik
			TCNT0 = 0; //zerowanie licznika timera0
			timerCounter_Global = 0; //zerowanie licznika mikrosekund
			echoRising_Global  = 0; //czekamy na nastêpny stan - to bêdzie opadajacy 			
		}else {
			TCCR0B &=~ (1<<CS00); //od³¹czamy zegar od timera 0
			if(((timerCounter_Global * 16) / 58) < 300)
			{
				switch (runningSonar_Global) //dwa czujniki dzia³aj¹
				{
					case 2:
						hcsr04_2.actualTime = (int)((timerCounter_Global * 16) / 58);
					break;
					
					case 3:
						hcsr04_3.actualTime = (int)((timerCounter_Global * 16) / 58);
					break;
				}
				
	/*			if(configRef_Global) {
					hcsr04_1.refTime = hcsr04_1.actualTime - 2;
					configRef_Global = false;
				}	*/
			} else {
				switch (runningSonar_Global)
				{
					case 2:
						hcsr04_2.actualTime = -1; //przeslij b³¹d do klasy czujnika 2
					break;
					
					case 3:
						hcsr04_3.actualTime = -1; //przeslij b³¹d -||- 3
					break;
				}
				
			}
			runningSonar_Global = 'F';		
		}
	}
}

/*	
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
				hcsr04_2.actualTime = (int)((timerCounter_Global * 16) / 58);				
//				if(configRef_Global) {
//					hcsr04_0.refTime = hcsr04_0.actualTime - 2;
//					configRef_Global = false;
				}	//
			} else {
				hcsr04_2.actualTime = -1;				
			}//else ustawiaj rezultat na -1czyli wywalaj b³¹d
			runningSonar_Global = 'F';		
		}
	}
}
*/

ISR(TIMER0_OVF_vect) //licznik czasu impulsów z czujnika 0 lub 1
{//TODO: MO¯E NIE DZIA£AÆ
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
	if(positionLedF_Global == 1)
	{
		if (tempLeft_Global != 0) PORTD &=~ tempLeft_Global; //hack 3
		if (tempRight_Global != 0) PORTD &=~ tempRight_Global; //hack 3
		

		if(OCR1A < OCR1B)
		{
			OCR1A += (OCR1B * 8) / (ledIncrease * 10);
		} else{
			tempLeft_Global <<= 1;
			tempRight_Global >>= 1;
			actualLedPin_Global |= tempLeft_Global|tempRight_Global; //hack 4
			
			if(actualLedPin_Global == 0xff /*&& (tempRight_Global < tempLeft_Global)*/)
			{
				static volatile uint8_t delay = 0;
				delay++;
				if(delay >= 2)
				{
					positionLedF_Global = 2; // odpala drugi modu³ 
					delay = 0;
					TIMSK1 &=~ (1<<OCIE1A); //WY£¥CZA PRZERWANIE KIEDY
				}
			}
			OCR1A = 0;
		}
	}		
}
ISR(TIMER1_COMPB_vect)
{	

 	switch(positionLedF_Global)
	{
		case 2:
		if (delayLedMS_Global > (1000 * ledTimeOn_Global))
		{
			positionLedF_Global = 3;
			delayLedMS_Global = 0;
		} else {
			delayLedMS_Global++;
		}
		case 1:
		PORTD &= ~actualLedPin_Global;
		break;
		
		case 3:
		PORTD &= ~actualLedPin_Global;
		if (OCR1B == 0)
		{
			positionLedF_Global = 0;
			tempLeft_Global = 0;
			tempRight_Global = 0;
			actualLedPin_Global = 0;
		} else {
			OCR1B -= 10;
		}

		break;
	}
}
ISR(TIMER1_OVF_vect)
{
	if(positionLedF_Global != 0)
	PORTD = actualLedPin_Global;
}

ISR(TWI_vect) //TODO: nie dzia³¹ 
{
	uint8_t status=TW_STATUS;
	static uint8_t index;
	switch(status)
	{
		case TW_SR_SLA_ACK:
			index = 0;
			i2cBufStatus.slaveTranciv = 0;
			i2cBufStatus.ready = 0;
		break;
		
		case TW_SR_DATA_ACK:
			i2cBufStatus.bufSlaveReciv[index] = TWDR;
			index++;
		break;
		
		
		case TW_ST_SLA_ACK:
			index = 0;
			i2cBufStatus.slaveTranciv = 1;
			i2cBufStatus.ready = 0;
			TWDR = i2cBufStatus.bufSlaveTranciv[index];
			index++;
		break;
		
		case TW_ST_DATA_ACK:
			TWDR = i2cBufStatus.bufSlaveTranciv[index];
			index++;
		break;
		
		
		
		case TW_SR_STOP:
			if (i2cBufStatus.slaveTranciv == 0)
			{
				i2cBufStatus.ready = 1;
				i2cConfigTranciv();
			} else 
			{
				i2cBufStatus.ready = 1;
			}
		break;
		
			
	}
	TWCR |= (1<<TWINT); // Zwolnij liniê SCL
}
/////////////////////////////////INT0 - CZUJNIK NR 0
/////////////////////////////////INT1 - CZUJNIK NR 1

