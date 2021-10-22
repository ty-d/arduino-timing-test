#include <Arduino.h>
#include <pin_defs.h>
#include <ArduinoModbus.h>
#include <modbus_registers.h>

enum State {
	Pulsing,
	Waiting,
    NotInitialized
};

struct Solenoids {
    State state;
    unsigned long lastUpdated;
    unsigned int currentSolenoid;
};

Solenoids newSolenoids() {
    return Solenoids {
        NotInitialized,
        0,
        0,
    };
}

void updateSolenoids(Solenoids& solenoids, unsigned long time) {
    if (solenoids.state == NotInitialized) {
        digitalWrite(SOLENOID_ARRAY[solenoids.currentSolenoid], HIGH);
        solenoids.state = Pulsing;
        solenoids.lastUpdated = time;
    } else if (solenoids.state == Pulsing) {
        int timeAllowed = 150;
#ifndef DEBUG
        timeAllowed = ModbusRTUServer.holdingRegisterRead(PulseOnTime);
#endif
        if (time - solenoids.lastUpdated > timeAllowed) {
            digitalWrite(SOLENOID_ARRAY[solenoids.currentSolenoid], LOW);
            solenoids.state = Waiting;
            solenoids.lastUpdated = time;
            solenoids.currentSolenoid = (solenoids.currentSolenoid + 1) % ModbusRTUServer.holdingRegisterRead(NumSolenoids);
        }
    } else if (solenoids.state == Waiting) {
        int timeAllowed = 1000;
#ifndef DEBUG
        timeAllowed = 1000 * ModbusRTUServer.holdingRegisterRead(PulseOffTime);
#endif
        if (time - solenoids.lastUpdated > timeAllowed) {
            digitalWrite(SOLENOID_ARRAY[solenoids.currentSolenoid], HIGH);
            solenoids.state = Pulsing;
            solenoids.lastUpdated = time;
        }
    }
}