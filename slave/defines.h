/*
 * defines.h
 *
 * Created: 26.07.2016 15:26:38
 */ 


#ifndef DEFINES_H_
#define DEFINES_H_



#define LED_0		PD0	//output
#define LED_1		PD1	//output
#define LED_2		PD2	//output
#define LED_3		PD3	//output
#define LED_4		PD4	//output
#define LED_5		PD5	//output
#define LED_6		PD6	//output
#define LED_7		PD7	//output

#define ECHO_3		PB0	//input 
#define TRIGGER_3	PB2	//output
//-------------------------

#define ECHO_2		PB1	//input	
#define TRIGGER_2	PB3	//output

#define Relay		PC3

//funkcje dla trigger



#define SONAR_TRIGGER3_LOW()	PORTB&=~(1<<TRIGGER_3)
#define SONAR_TRIGGER3_HIGH()	PORTB|=(1<<TRIGGER_3)

#define SONAR_TRIGGER2_LOW()	PORTB&=~(1<<TRIGGER_2)
#define SONAR_TRIGGER2_HIGH()	PORTB|=(1<<TRIGGER_2)

#define RELAY_ON()				PORTC |= (1<<Relay)
#define RELAY_OFF()				PORTC &=~ (1<<Relay)

#define MAX_RESP_TIME_MS	200
#define DELAY_BTW_TESTS_MS	100

#define slave_adress 174

#endif /* DEFINES_H_ */
