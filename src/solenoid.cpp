#include <solenoid.h>

//extern ModbusTCPServer modbusTCPServer;

Solenoids newSolenoids() {
    return Solenoids {
        WaitingForHigh,
        0,
        0,
    };
}

void updateSolenoids(Solenoids& solenoids, unsigned long time, float pressure, bool override) {
    if (solenoids.state == WaitingForHigh) {
        if ((pressure >= readModbusFloat(HighLimit)) || override) {
            digitalWrite(SOLENOID_ARRAY[solenoids.currentSolenoid], HIGH);
            solenoids.state = Pulsing;
            solenoids.lastUpdated = time;
        }
    } else if (solenoids.state == Pulsing) {
        int timeAllowed = 150;
#ifndef DEBUG
        timeAllowed = modbusTCPServer.holdingRegisterRead(PulseOnTime);
#endif
        if (time - solenoids.lastUpdated > timeAllowed) {
            digitalWrite(SOLENOID_ARRAY[solenoids.currentSolenoid], LOW);
            solenoids.state = Waiting;
            solenoids.lastUpdated = time;
            solenoids.currentSolenoid = (solenoids.currentSolenoid + 1) % modbusTCPServer.holdingRegisterRead(NumSolenoids);
        }
    } else if (solenoids.state == Waiting) {
        int timeAllowed = 1000;
#ifndef DEBUG
        timeAllowed = 1000 * modbusTCPServer.holdingRegisterRead(PulseOffTime);
#endif
        if (time - solenoids.lastUpdated > timeAllowed) {
            if (!override && (pressure <= readModbusFloat(LowLimit))) {
                solenoids.state = WaitingForHigh;
            } else {
                digitalWrite(SOLENOID_ARRAY[solenoids.currentSolenoid], HIGH);
                solenoids.state = Pulsing;
                solenoids.lastUpdated = time;
            }
        }
    }
}