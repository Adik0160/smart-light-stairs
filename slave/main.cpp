/*
 * slave.cpp
 *
 * Created: 2016-07-09 00:37:41
 * Author : Adikoff
 */ 

#include <avr/io.h>
#include <util/delay.h>
#include <avr/interrupt.h>

int main(void)
{
	DDRB = 0;
	DDRD = 0xff;
	PCICR |= (1<<PCIE0);
	PCIFR = (1<<PCIF0);
	PCMSK0 = (1<<PCINT0);
	


	sei();
	

    while (1) 
    {

    }
}

ISR(PCINT0_vect)
{
	PORTD ^= 0xff;
	
}