#include <Arduino.h>

/*
    NOTE: all of these are far from final, just random numbers for now
*/

#ifndef INCLUDED_PIN_DEFS
#define INCLUDED_PIN_DEFS

// Inputs
const int PRESSURE_SENSOR_INPUT = 3;
const int DOWNTIME_CLEANING_INPUT = 4;
const int MANUAL_CLEANING_INPUT = 5;
const int HIGH_ALARM_RESET = 6;
const int MANUAL_OVERRIDE = 7;
const int OPERATION_RESET = 8;

// Button inputs
const int VALUE_UP = 22;
const int VALUE_DOWN = 24;
const int SELECT = 26;

void setupTimingInputs() {
    pinMode(PRESSURE_SENSOR_INPUT, INPUT);
    pinMode(DOWNTIME_CLEANING_INPUT, INPUT);
    pinMode(MANUAL_CLEANING_INPUT, INPUT);
    pinMode(HIGH_ALARM_RESET, INPUT);
    pinMode(MANUAL_OVERRIDE, INPUT);
    pinMode(OPERATION_RESET, INPUT);
    pinMode(VALUE_UP, INPUT);
    pinMode(VALUE_DOWN, INPUT);
    pinMode(SELECT, INPUT);
}

// Outputs
const int SOLENOID_ARRAY[] = {3, 4, 5, 6, 36, 8};
const int MAX_NUM_SOLENOIDS = 40;

const int HIGH_ALARM = 10;
const int MAINTENANCE_ALARM = 11;
const int PRESSURE_TRANSMITTER = 12;

void setupTimingOutputs() {
	for (int s = 0; s < MAX_NUM_SOLENOIDS; s++) {
		pinMode(SOLENOID_ARRAY[s], OUTPUT);
	}
    pinMode(HIGH_ALARM, OUTPUT);
    pinMode(MAINTENANCE_ALARM, OUTPUT);
    pinMode(PRESSURE_TRANSMITTER, OUTPUT);
}

#endif