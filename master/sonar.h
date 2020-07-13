/*
 * sonar.h
 *
 * Created: 26.07.2016 15:05:37
 *  Author: Adikoff
 */ 


#ifndef SONAR_H_
#define SONAR_H_

extern volatile uint8_t runningSonar_Global; //F - false - trwanie jakiegoœ pomiaru
extern volatile uint8_t echoRising_Global;
//extern volatile uint8_t configRef_Global; //przy konfiguracji czujnika musi byæ true

class Sonar
{ 
	public:
	volatile int actualTime; //odleg³oœæ w cm
	volatile uint8_t compareFlag;
	
	private:
	int refTime; //tutaj równie¿ - zawsze mniejsza o 2 cm ni¿ actuakTime
	int avgBuf[5]; //bufor piêciu pomiarów - u¿ywane w funkcji avarage_config()
	uint8_t sonarNumber; //numer czujnika !!WA¯NE
	
	public:
	Sonar(uint8_t, int);	//konstruktor -- pierwsza liczba: nr czujnika -druga: odleg³oœæ referenycjna
	int measure(uint8_t, uint8_t); //configRef = 'true' kiedy w³¹czamy konfiguracje czujnika - 'false' przy zwyk³ym skanowaniu
	uint8_t comparate(uint8_t); //je¿eli zwróci 1 to znaczy ¿e jest mniejsze od czasu referencyjnego, je¿eli nie to 0	
	int avarage_config(void); //funkcja wykonuje 5 pomiarów, usuwa liczby skrajne. Z trzech robi œredni¹, odejmuje 2 cm i zwraca j¹ po przez wartoœæ oraz zapisuje do zmiennej klasy refTime
};

class Extreme //klasa extermum
{
	public:
	int value; //wartoœæ
	uint8_t index; //index wartoœi - brana z "i" pêtli for
	
	Extreme(uint8_t, int);	//konstruktor
	void upload(uint8_t, int); 
};

#endif /* SONAR_H_ */
