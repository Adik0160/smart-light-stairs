/*
 * i2c_slave.cpp
 *
 * Created: 19.09.2016 00:49:59
 */ 

#include <avr/io.h>
#include <stdio.h>
#include <stdlib.h>
#include <util/twi.h>

#include "defines.h"
#include "sonar.h"


void i2c_init(void)
{		
	TWAR = slave_adress;
	TWCR |= (1<<TWEA)|(1<<TWEN)|(1<<TWIE)|(1<<TWINT);
}
