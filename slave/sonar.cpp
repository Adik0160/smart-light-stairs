/*
 * sonar.cpp
 *
 * Created: 26.07.2016 15:05:25
 */ 
#include <avr/io.h>
#include <util/delay.h>
#include <stdio.h>
#include <stdlib.h>
#include "defines.h"
#include "sonar.h"



int Sonar::measure(uint8_t configRef = false, uint8_t waitingFlag = true) //configRef = 'true' kiedy w³¹czamy konfiguracje czujnika - 'false' przy zwyk³ym skanowaniu
{
	switch (sonarNumber){
		case 2:
		SONAR_TRIGGER2_HIGH();
		break;
		case 3:
		SONAR_TRIGGER3_HIGH();
		break;
		default:
		refTime = -1;
		actualTime = -1; //je¿eli nie bêdzie zaindeksowanego czujnika na 0 lub 1 to wywal b³¹d
		return -1;
		break;
	}
		
//	configRef_Global = configRef;//configRef = 1 kiedy w³¹czamy konfiguracje czujnika - 0 przy zwyk³ym skanowanius
	runningSonar_Global = sonarNumber; //flaga czujnika - mo¿na odczytywaæ, je¿eli równe 0 = czujnik 0 oraz przerwanie 0, je¿eli 1 = czujnik 1 oraz przerwanie 1, a je¿eli 'F' to blokada przerwania
	echoRising_Global = 1;	//w³¹czenie oczekiwania na stan narastaj¹cy na przerwaniu
	_delay_us(10);
	switch (sonarNumber){
		case 2:
		SONAR_TRIGGER2_LOW();
		break;
		case 3:
		SONAR_TRIGGER3_LOW();
		break;
	}
	uint32_t x = 0;
	while((runningSonar_Global != 'F') && waitingFlag) if(x >= 2000000UL) break; else x++;
	if(configRef && actualTime != -1) //wstêpne zabezpieczenie przed wpisaniem kodu b³êdu do zmiennej refTime
	{
		refTime = actualTime - 2; //odejmujemy 2 cm od zmiennej actualTime dla bezpieczeñstwa
	}
	return actualTime;
}
	
uint8_t Sonar::comparate(uint8_t waitingFlag = true) //je¿eli zwróci 1 to znaczy ¿e jest mniejsze od czasu referencyjnego, je¿eli nie to 0
{
	uint32_t x = 0;
	while((runningSonar_Global != 'F') && waitingFlag) if(x >= 2000000UL) break; else x++;  //je¿eli trwa jakiœ pomiar - czekaj
	measure();
	x = 0;
	while((runningSonar_Global != 'F') && waitingFlag) if(x >= 2000000UL) break; else x++; //czekaj na skoñczenie pomiaru, póŸniej porównuj
//	if (actualTime < refTime)
//	{
//		compareFlag = 1;
//	}
	return actualTime < refTime ? 1 : 0; //zwracaj 1 kiedy aktualny czas jest mniejszy od czasu referecyjnego, je¿eli nie zwracaj 0
}
	
int Sonar::avarage_config(void)
{
	int measureTemp;
	for ( uint8_t i = 0 ; i < 5 ; i++ ) //dokonywanie piêciu pomiarów i zapisywanie ich w pamiêci
	{
		measureTemp = measure(); //wykonywanie pomiaru i zwracanie go do tablicy
		if( measureTemp == -1 ){
			i--;
		}else{
			avgBuf[i] = measureTemp;
		}
		_delay_ms(DELAY_BTW_TESTS_MS);
	}
	Extreme extremeBottom(0,avgBuf[0]); //definiowanie i deklarowanie obiektów extremów dolnych i górnych
	Extreme extremeTop(1,avgBuf[1]);
	
	for (uint8_t i = 0; i<5; i++) //znajodwanie skrajnych odleg³osci i wypisywanie ich do indeksu
	{
		if(avgBuf[i] < extremeBottom.value) extremeBottom.upload(i,avgBuf[i]);
		if(avgBuf[i] > extremeTop.value) extremeTop.upload(i,avgBuf[i]);
	}

	int sum = 0;
	
	for (uint8_t i = 0; i<5; i++) //dodawanie liczb - dwa zawsze nie zostan¹ dodane
	{
		if((i != extremeBottom.index) && (i != extremeTop.index))
		{
			sum += avgBuf[i]; //sumowanie liczb które nie s¹ skrajne
		}
	}
	refTime = sum / 3 - 2; //wyliczamy œredni¹ i odejmujemy od niej 2 cm
	return refTime; //przy okazji zwracamy czas referenycjny
}

Sonar::Sonar(uint8_t sonarNr,int refT = 0) //konstruktor -- domyœlny czas referencyjny bêdzie wynosi³ 0cm - nr czunika jest obowiazkowy!!
{
	sonarNumber = sonarNr;
	refTime = refT;
//	compareFlag = 0;
}

/*
				if(configRef_Global) {
					hcsr04_1.refTime = hcsr04_1.actualTime - 2;
					configRef_Global = false;
*/

Extreme::Extreme(uint8_t i, int v)
{
	index = i;
	value = v;
}

void Extreme::upload(uint8_t i, int v)
{
	index = i;
	value = v;
}

