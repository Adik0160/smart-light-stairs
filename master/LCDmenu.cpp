/* 
* LCDmenu.cpp
*
* Created: 20.09.2016 19:57:48
*/
#include <avr/io.h>
#include <stdio.h>
#include <stdlib.h>
#include <util/delay.h>
#include <string.h>

#include "LCDmenu.h"
#include "HD44780.h"
#include "defines.h"

// default constructor
void LCDmenu::constructor(uint8_t i, char* t, char* t2, uint8_t vm, uint8_t s, uint8_t vmin)
{
	index = i;
	text = t;
	text2 = t2;
	valueMax = vm;
	valueMin = vmin;
	step = s;
} //LCDmenu


void LCDmenu::letMeShowYouHowItsDone(void)
{
	LCD_Home();
	LCD_Clear();
	LCD_GoTo(0,0);
	LCD_WriteText(text);
	LCD_GoTo(0,1);
	itoa(value,temp,10);
	if (step == 1) strcat(temp,(char*)"0");
	LCD_WriteText(temp);
	LCD_WriteText(text2);
	_delay_ms(300);
}

void LCDmenu::valueUp(void)
{
	_delay_ms(30);
	value++;
	if (value > valueMax || value < valueMin) value = valueMin;
	itoa(value,temp,10);
	if (step == 1) strcat(temp,(char*)"0");
	strcat(temp, text2);
	LCD_Clear();
	LCD_GoTo(0,0);
	LCD_WriteText(text);
	LCD_GoTo(0,1);
	LCD_WriteText(temp);
	
	
}
 void LCDmenu::valueDown(void)
 {
	_delay_ms(30);
	value--;
	if (value > valueMax || value < valueMin) value = valueMax;
	itoa(value,temp,10);
	if (step == 1) strcat(temp,(char*)"0");
	strcat(temp, text2);
	LCD_Clear();
	LCD_GoTo(0,0);
	LCD_WriteText(text);
	LCD_GoTo(0,1);
	LCD_WriteText(temp);
 }
