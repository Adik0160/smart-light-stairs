/* 
* LCDmenu.h
*
* Created: 20.09.2016 19:57:48
*/

#include <avr/io.h>
#include <stdio.h>
#include <stdlib.h>

#ifndef __LCDMENU_H__
#define __LCDMENU_H__


class LCDmenu
{
//variables
public:
	uint8_t index;
	char* text;
	char* text2;
	
	unsigned char value;
	void constructor(uint8_t, char*, char*, uint8_t, uint8_t, uint8_t);
	void letMeShowYouHowItsDone(void);
	void valueUp(void);
	void valueDown(void);
private:
	char temp[16];
	uint8_t valueMax;
	uint8_t valueMin;
	uint8_t step;
}; 

#endif 
