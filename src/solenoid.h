#ifndef SOLENOID_INCLUDED
#define SOLENOID_INCLUDED

#include <Arduino.h>
#include <pin_defs.h>
#include <ArduinoModbus.h>
#include <modbus_registers.h>

enum State {
	Pulsing,
	Waiting,
    WaitingForHigh,
};

struct Solenoids {
    State state;
    unsigned long lastUpdated;
    unsigned int currentSolenoid;
};

Solenoids newSolenoids();

void updateSolenoids(Solenoids& solenoids, unsigned long time, float pressure);

#endif