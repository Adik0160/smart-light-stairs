/*
 * sonar.h
 *
 * Created: 26.07.2016 15:05:37
 *  Author: Adikoff
 */ 


#ifndef SONAR_H_
#define SONAR_H_

extern volatile uint8_t runningSonar_Global; //F - false - trwanie jakiego� pomiaru
extern volatile uint8_t echoRising_Global;
//extern volatile uint8_t configRef_Global; //przy konfiguracji czujnika musi by� true

class Sonar
{ 
	public:
	volatile int actualTime; //odleg�o�� w cm
	volatile uint8_t compareFlag;
	
	private:
	int refTime; //tutaj r�wnie� - zawsze mniejsza o 2 cm ni� actuakTime
	int avgBuf[5]; //bufor pi�ciu pomiar�w - u�ywane w funkcji avarage_config()
	uint8_t sonarNumber; //numer czujnika !!WA�NE
	
	public:
	Sonar(uint8_t, int);	//konstruktor -- pierwsza liczba: nr czujnika -druga: odleg�o�� referenycjna
	int measure(uint8_t, uint8_t); //configRef = 'true' kiedy w��czamy konfiguracje czujnika - 'false' przy zwyk�ym skanowaniu
	uint8_t comparate(uint8_t); //je�eli zwr�ci 1 to znaczy �e jest mniejsze od czasu referencyjnego, je�eli nie to 0	
	int avarage_config(void); //funkcja wykonuje 5 pomiar�w, usuwa liczby skrajne. Z trzech robi �redni�, odejmuje 2 cm i zwraca j� po przez warto�� oraz zapisuje do zmiennej klasy refTime
};

class Extreme //klasa extermum
{
	public:
	int value; //warto��
	uint8_t index; //index warto�i - brana z "i" p�tli for
	
	Extreme(uint8_t, int);	//konstruktor
	void upload(uint8_t, int); 
};

#endif /* SONAR_H_ */
