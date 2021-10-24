/*
 * master.cpp
 * ATmega32 16MHz
 * Created: 2016-07-09 00:37:01
 */ 



#include <avr/io.h>
#include <util/delay.h>
#include <avr/interrupt.h>
#include <stdio.h>
#include <stdlib.h>
#include <util/twi.h>

#include "defines.h"
#include "HD44780.h"
#include "sonar.h"
#include "i2c_master.h"
#include "LCDmenu.h"

//

///////---ZMIENNE GLOBALNE UWAGA!---////////////
volatile uint8_t runningSonar_Global = 'F'; //F - false - trwanie jakiegoœ pomiaru
volatile uint8_t echoRising_Global = false;
//volatile uint8_t configRef_Global = false; //przy konfiguracji czujnika musi byæ true
volatile uint32_t timerCounter_Global = 0;
volatile uint8_t actualLedPin_Global = 0;
volatile uint8_t positionLedF_Global = 0; //stage
volatile uint16_t delayLedMS_Global = 0;

uint8_t relayTime_Global = 30; // [min]
uint8_t ledTimeOn_Global = 5; // [sec]
uint8_t ledIncrease = 200;//0   [ms]
class ledBrightConfig{ //TODO: ogarnaæ t¹ klase i to równaniedo czujnika œwiat³a    (valueMax - valueMin) * actualFromADC
	public:										//									--------------------------------------  + valueMin = ledBright
	uint8_t ledBright;													//					255(albo 256 nie wiem)
	uint8_t	valueMax;
	uint8_t valueMin;
	uint8_t actualFromADC;
	ledBrightConfig(uint8_t lb, uint8_t vma, uint8_t vmi, uint8_t afa)
	{
		ledBright  = lb;
		valueMax = vma;
		valueMin = vmi;
		actualFromADC = afa;
	}
	
	void ledBrighGenerate(void)
	{
		char tmp[5];
		ledBright = ((valueMax - valueMin) * actualFromADC / 255) + valueMin;
		itoa(ledBright, tmp, 10);
		LCD_Clear();
		LCD_Home();
		LCD_WriteText(tmp);
	}
};

ledBrightConfig ledBrightConfig(100, 100, 1, 255);

volatile uint8_t tempLeft_Global = 0;
volatile uint8_t tempRight_Global = 0; //hack 2
////////////////////////////////////////////////

enum position {up, down, both};



Sonar hcsr04_0(0,0);
Sonar hcsr04_1(1,0);
Sonar_Slave hcsr04_2(2);
Sonar_Slave hcsr04_3(3);
LCDmenu menu[6];


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


void sonars_config(uint8_t slaveSonarsConfig) //je¿eli zero to nie wysy³aj roskazu do slejva
{
	if(slaveSonarsConfig == 1)
	{
		i2c_start(slave_adress | TW_WRITE);
		i2c_write('3');
		i2c_stop();
	}
	
	char temp[16];
	LCD_LED_ON();
	LCD_Home();
	LCD_Clear();
	LCD_GoTo(1,0);
	LCD_WriteText((char*)"CZUJNIK NR. 1");
	sprintf(temp ,"Odl REF: %dcm" ,hcsr04_0.avarage_config());
	LCD_GoTo(1,1);
	LCD_WriteText(temp);
	_delay_ms(1000);
	
	LCD_Clear();
	LCD_GoTo(1,0);
	LCD_WriteText((char*)"CZUJNIK NR. 2");
	sprintf(temp ,"Odl REF: %dcm" ,hcsr04_1.avarage_config());
	LCD_GoTo(1,1);
	LCD_WriteText(temp);
	
	_delay_ms(1000);
	i2c_start(slave_adress | TW_WRITE);
	i2c_write('#');
	i2c_start(slave_adress | TW_READ);
	hcsr04_2.refTime = i2c_read_ack();
	hcsr04_3.refTime = i2c_read_nack();
	i2c_stop();
	
	LCD_Clear();
	LCD_GoTo(1,0);
	LCD_WriteText((char*)"CZUJNIK NR. 3");
	sprintf(temp ,"Odl REF: %dcm" ,hcsr04_2.refTime);
	LCD_GoTo(1,1);
	LCD_WriteText(temp);
		
	_delay_ms(1000);
		
	LCD_Clear();
	LCD_GoTo(1,0);
	LCD_WriteText((char*)"CZUJNIK NR. 4");
	sprintf(temp ,"Odl REF: %dcm" ,hcsr04_3.refTime);
	LCD_GoTo(1,1);
	LCD_WriteText(temp);
	_delay_ms(1000);

	
	LCD_LED_OFF();
	LCD_Clear();	

} 

void port_init(void) //inicjalizacja portów 
{
	DDRA &=~ (1<<BUTTON_1)|(1<<BUTTON_2)|(1<<BUTTON_3)|(1<<BUTTON_4); //input
	DDRA |= (1<<Relay);
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
	
	
	ICR1 = 16001;
	TCCR1A |= (1<<WGM11); //TIMER 1 MODE 14
	TCCR1B |= (1<<WGM13)|(1<<WGM12); //PRESKALLER 1 1kHz
}

void ex_interrupt_init(void)
{
	MCUCR |= (1<<ISC10)|(1<<ISC00); //wywo³ywanie przerwania przy kia¿dym zboczu - zewnêtrzne przerwanie int0
	GICR |= (1<<INT1)|(1<<INT0);	//w³aczenie przerwania int0s
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
		OCR1B = (16000 * ledBrightConfig.ledBright)/100; //16 tyœ, max wartoœæ
		positionLedF_Global = 1;
			
		TIMSK |= (1<<OCIE1A);
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

void ADC_start_converting(void)
{
	i2c_start(slave_adress | TW_WRITE);
	i2c_write('!');
	i2c_stop();
	_delay_ms(400);
	i2c_start(slave_adress | TW_WRITE);
	i2c_write('F');
	i2c_start(slave_adress | TW_READ);
	ledBrightConfig.actualFromADC = i2c_read_nack();
	i2c_stop();	
	LCD_Home();
	ledBrightConfig.ledBrighGenerate();
}



void mainMenu(void) //TODO: naprawiæ to kurestwo 
{
	menu[0].constructor(0,(char*)"Czas przekaznika",(char*)" min", 255, 0, 1);
	menu[0].value = relayTime_Global;
	menu[1].constructor(1,(char*)"Czas swiecenia",(char*)" sec", 255, 0, 0);
	menu[1].value = ledTimeOn_Global;
	menu[2].constructor(3,(char*)"Czas zaswiecania",(char*)" ms", 255, 1, 11);
	menu[2].value = ledIncrease;
	menu[3].constructor(4,(char*)"Kalibracja sens.",(char*)"", 1, 0, 0);
	menu[3].value = 0;
	menu[4].constructor(4,(char*)"Jasnosc MAX",(char*)" %", 100, 0, 1);
	menu[4].value = ledBrightConfig.valueMax;
	menu[5].constructor(4,(char*)"Jasnosc MIN",(char*)" %", 100, 0, 1);
	menu[5].value = ledBrightConfig.valueMin;
	
	uint8_t prog = 0;
	menu[prog].letMeShowYouHowItsDone();
	_delay_ms(600);
	while (1)
	{
		if (BUTTON_4_PRESSED())
		{
			prog++;
			if (prog == 6) prog = 0;
			menu[prog].letMeShowYouHowItsDone();			
		}
		
		if (BUTTON_1_PRESSED())
		{
			LCD_Clear();
			relayTime_Global = menu[0].value;
			ledTimeOn_Global = menu[1].value;
			ledIncrease = menu[2].value;
			ledBrightConfig.valueMax = menu[4].value;
			ledBrightConfig.valueMin = menu[5].value;
			
			i2c_start(slave_adress | TW_WRITE);
			i2c_write('C');
			i2c_write(relayTime_Global);
			i2c_write(ledTimeOn_Global);
			i2c_write(ledIncrease);
			i2c_write(ledBrightConfig.valueMax);
			i2c_write(ledBrightConfig.valueMin);
			i2c_stop();
			
			LCD_WriteText((char*)"Zapisano w RAM");
			_delay_ms(600);
			LCD_Clear();
			break;
		}
		
		if (BUTTON_2_PRESSED())
		{
			menu[prog].valueUp();
			_delay_ms(20);
			if(prog == 4)
			{
				_delay_ms(300);
				sonars_config(1);
				menu[4].value = 0;
				menu[4].letMeShowYouHowItsDone();
			}

		}
		
		if (BUTTON_3_PRESSED())
		{
			if (prog == 4) menu[prog].valueUp();
			else menu[prog].valueDown();
			_delay_ms(20);
			if(prog == 4) 
			{
				_delay_ms(300);
				sonars_config(1);
				menu[4].value = 0;
				menu[4].letMeShowYouHowItsDone();	
			}

		}
	}
}



int main(void)
{
	uint8_t relayTimeCompare = 0;
	uint8_t luxSensorTCompare = 0;
	timer_init();
	_delay_ms(100);
	port_init();
	ex_interrupt_init();
	i2c_init();	
	sei();
	RELAY_ON();	
	sonars_config(0);
	ADC_start_converting();
	char temp[16];
	
    for (;;) 
    {	///test i2c

		
		//koniec testu
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
		
			if (relayTimeCompare >= relayTime_Global*200) RELAY_OFF(); else relayTimeCompare++;
			if (luxSensorTCompare >= 33 && positionLedF_Global == 0) 
			{
				ADC_start_converting();
				luxSensorTCompare = 0;
			} else 
			{
				luxSensorTCompare++;
			}

 			if (hcsr04_0.comparate(true) && hcsr04_0.actualTime != -1)
 			{
				RELAY_ON();
 				start_lighting(up);
				sprintf(temp ,"%dcm" ,hcsr04_0.actualTime);
				LCD_GoTo(0,0);
				LCD_WriteText(temp);
				relayTimeCompare = 0;
 			}
 			if (hcsr04_1.comparate(true) && hcsr04_1.actualTime != -1)
 			{
				 RELAY_ON();
 				start_lighting(down);
				 sprintf(temp ,"%dcm" ,hcsr04_1.actualTime);
				 LCD_GoTo(0,1);
				 LCD_WriteText(temp);
				 relayTimeCompare = 0;
 			}
			
			for (int x = 0; x < 300 ;x++)
			{
				if (BUTTON_1_PRESSED())
				{
					mainMenu();
				}
				_delay_ms(1);
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


	if(positionLedF_Global == 1)
	{
		if (tempLeft_Global != 0) PORTB &=~ tempLeft_Global; //hack 3
		if (tempRight_Global != 0) PORTB &=~ tempRight_Global; //hack 3
		

		if(OCR1A < OCR1B)
		{
			OCR1A += (OCR1B * 8) / (ledIncrease * 10);
		} else{
			PORTB = 0xff;/////////////////////////////////////////////////////////////////////////////
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
					TIMSK &=~ (1<<OCIE1A); //WY£¥CZA PRZERWANIE KIEDY
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
		PORTB &= ~actualLedPin_Global;
		break;
		
		case 3:
		PORTB &= ~actualLedPin_Global;
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
	PORTB = actualLedPin_Global;
}



/////////////////////////////////INT0 - CZUJNIK NR 0
/////////////////////////////////INT1 - CZUJNIK NR 1

