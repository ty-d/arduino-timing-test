#include <pin_defs.h>

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

void setupTimingOutputs() {
	for (int s = 0; s < MAX_NUM_SOLENOIDS; s++) {
		pinMode(SOLENOID_ARRAY[s], OUTPUT);
	}
    pinMode(HIGH_ALARM, OUTPUT);
    pinMode(MAINTENANCE_ALARM, OUTPUT);
    pinMode(PRESSURE_TRANSMITTER, OUTPUT);
}
