#include <pressure.h>

Adafruit_ADS1115 pressureReader;

float readPressureSensor() {
    // voltage ranges from 0 to 4.46, 0 pressure at around 0.75
    int16_t something = pressureReader.readADC_SingleEnded(0);
    float volts = pressureReader.computeVolts(something);
    return (volts - 0.75) * (10.00 / (4.46 - 0.75));
}