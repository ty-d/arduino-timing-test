#include <Arduino.h>
#include <fsm.h>

int pinNum = 13;

void setup() {
	pinMode(pinNum, OUTPUT);
}

void loop() {
	add(5, 6); // just testing

	delay(5000);
	digitalWrite(pinNum, HIGH);
	delay(150);
	digitalWrite(pinNum, LOW);
}