#include <Arduino.h>
#include <persistent.h>
#include <pin_defs.h>
#include <EEPROM.h>
#include <timer.h>
#include <ArduinoModbus.h>
#include <solenoid.h>
#include <debug.h>
#include <modbus_registers.h>

// Global variables
PersistentVals persistentVals;
UserSettings userSettings;

Timer operationTimer;
Timer lifetimeTimer;
Timer highAlarmTimer;
Timer cleaningTimer;

const int PULSE_CYCLE_MAINTENANCE_THRESHOLD = 1000; // What should this value be? is it constant?
unsigned long lastEEPROMWrite = 0;

Solenoids solenoids = newSolenoids();

void setup() {
#ifdef DEBUG
	Serial.begin(9600);
#endif

	DEBUG_PRINT("Serial debugging enabled");

	// get persistent data from eeprom
	EEPROMWrapper wrapper;
	EEPROM.get(0, wrapper);

	if (wrapper.written != EEPROM_WRITTEN_CONST) {
		DEBUG_PRINT("using defaults");
		persistentVals = newPersistentVals();
		userSettings = defaultSettings();
	} else {
		DEBUG_PRINT("using current");
		persistentVals = wrapper.persistentVals;
		userSettings = wrapper.userSettings;
	}

	setupTimingInputs();
	setupTimingOutputs();

	modbusInit(wrapper.userSettings);

	// Assume these start for now
	operationTimer = newTimer(persistentVals.operationTime);
	startTimer(operationTimer, millis());
	lifetimeTimer = newTimer(persistentVals.lifeTime);
	startTimer(lifetimeTimer, millis());
	DEBUG_PRINT("Total lifetime (seconds):");
	DEBUG_PRINT(lifetimeTimer.totalTime);

	highAlarmTimer = newTimer(0);
	cleaningTimer = newTimer(0);
}

void loop() {

#ifndef DEBUG
	ModbusRTUServer.poll();
#endif

	unsigned long currentTime = millis();

	// Manage EEPROM
	if ((currentTime - lastEEPROMWrite > 10000) || (currentTime < lastEEPROMWrite)) {
		persistentVals.operationTime = elapsedTime(operationTimer, millis());
		persistentVals.lifeTime = elapsedTime(lifetimeTimer, millis());
		DEBUG_PRINT("Updated lifetime (sec):");
		DEBUG_PRINT(persistentVals.lifeTime);

		lastEEPROMWrite = currentTime;
		// NOTE: needs to be changed, this is a simple solution that doesn't have good enough endurance (100,000 writes guaranteed on EEPROM)
		EEPROM.put(0x00, EEPROMWrapper {
			EEPROM_WRITTEN_CONST, persistentVals, modbusGetUserSettings()
		});
		DEBUG_PRINT(lastEEPROMWrite);
	}

	updateSolenoids(solenoids, currentTime);
}