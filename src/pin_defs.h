#ifndef INCLUDED_PIN_DEFS
#define INCLUDED_PIN_DEFS

#include <Arduino.h>

/*
    NOTE: all of these are far from final, just random numbers for now
*/

// Inputs
const int DOWNTIME_CLEANING_INPUT = A8;
const int MANUAL_CLEANING_INPUT = A9;
const int HIGH_ALARM_RESET = A10;
const int MANUAL_OVERRIDE = A12;
const int OPERATION_RESET = A11;

// Button inputs
const int VALUE_UP = A13;
const int VALUE_DOWN = A14;
const int SELECT = A15;

void setupTimingInputs();

// Outputs
const int SOLENOID_ARRAY[] = {38, 40, 42, 44, 46, 48};
const int MAX_NUM_SOLENOIDS = 6;

const int HIGH_ALARM = 45;
const int MAINTENANCE_ALARM = 47;

void setupTimingOutputs();

#endif