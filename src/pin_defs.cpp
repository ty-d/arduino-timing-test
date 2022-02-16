#include <pin_defs.h>

void setupTimingInputs() {
    pinMode(DOWNTIME_CLEANING_INPUT, INPUT_PULLUP);
    pinMode(MANUAL_CLEANING_INPUT, INPUT_PULLUP);
    pinMode(HIGH_ALARM_RESET, INPUT_PULLUP);
    pinMode(MANUAL_OVERRIDE, INPUT_PULLUP);
    pinMode(OPERATION_RESET, INPUT_PULLUP);
    pinMode(VALUE_UP, INPUT_PULLUP);
    pinMode(VALUE_DOWN, INPUT_PULLUP);
    pinMode(SELECT, INPUT_PULLUP);
}

void setupTimingOutputs() {
	for (int s = 0; s < MAX_NUM_SOLENOIDS; s++) {
		pinMode(SOLENOID_ARRAY[s], OUTPUT);
	}
    pinMode(HIGH_ALARM, OUTPUT);
    pinMode(MAINTENANCE_ALARM, OUTPUT);
}
