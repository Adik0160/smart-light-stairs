/*
 * defines.h
 *
 * Created: 26.07.2016 15:26:38
 */ 


#ifndef DEFINES_H_
#define DEFINES_H_

#define BUTTON_1	PA0	//input
#define BUTTON_2	PA1	//input
#define BUTTON_3	PA2	//input
#define BUTTON_4	PA3	//input

#define LED_0		PB0	//output
#define LED_1		PB1	//output
#define LED_2		PB2	//output
#define LED_3		PB3	//output
#define LED_4		PB4	//output
#define LED_5		PB5	//output
#define LED_6		PB6	//output
#define LED_7		PB7	//output

#define ECHO_1		PD3	//input ---------INT1------
#define TRIGGER_1	PD5	//output----czujnik nr 1---
//-------------------------

#define ECHO_0		PD2	//input	---------INT0-------
#define TRIGGER_0	PD4	//output----czujnik nr 0----
//--------------------------
#define	LCD_LED		PD6	//output

#define Relay		PA4

//funkcje dla trigger, leda w lcd, przycisków etc

#define BUTTON_1_PRESSED()		!(PINA&(1<<BUTTON_1))
#define BUTTON_2_PRESSED()		!(PINA&(1<<BUTTON_2))
#define BUTTON_3_PRESSED()		!(PINA&(1<<BUTTON_3))
#define BUTTON_4_PRESSED()		!(PINA&(1<<BUTTON_4))

#define SONAR_TRIGGER1_LOW()	PORTD&=~(1<<TRIGGER_1)
#define SONAR_TRIGGER1_HIGH()	PORTD|=(1<<TRIGGER_1)

#define SONAR_TRIGGER0_LOW()	PORTD&=~(1<<TRIGGER_0)
#define SONAR_TRIGGER0_HIGH()	PORTD|=(1<<TRIGGER_0)

#define LCD_LED_ON()			PORTD |= (1<<LCD_LED)
#define LCD_LED_OFF()			PORTD &=~ (1<<LCD_LED)

#define RELAY_ON()				PORTA |= (1<<Relay)
#define RELAY_OFF()				PORTA &=~ (1<<Relay)

//sta³e
#define MAX_RESP_TIME_MS	200
#define DELAY_BTW_TESTS_MS	100

#define slave_adress 174


#endif /* DEFINES_H_ */
