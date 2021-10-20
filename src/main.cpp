#include <Arduino.h>
#include <persistent.h>
#include <pin_defs.h>
#include <EEPROM.h>
#include <timer.h>
#include <ArduinoModbus.h>
#include <solenoid.h>
#include <debug.h>
#include <modbus_registers.h>
#include <SevSeg.h>
#include <input_handler.h>

// Global variables
PersistentVals persistentVals;
UserSettings userSettings;

Timer operationTimer;
Timer lifetimeTimer;
Timer highAlarmTimer;
Timer cleaningTimer;

const unsigned long PULSE_CYCLE_MAINTENANCE_THRESHOLD = 1000000;
unsigned long lastEEPROMWrite = 0;

Solenoids solenoids = newSolenoids();

InputHandler inputHandler = newInputHandler();
ButtonDebouncer select = newButtonDebouncer();
ButtonDebouncer valueDown = newButtonDebouncer();
ButtonDebouncer valueUp = newButtonDebouncer();

SevSeg display;

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
	
	// pin 1 to a5 E, 2 a4 D, 3 a3 DP, 4 a2 C, 5 a1 G
	// pin 13 to 7 B
	// 8 a0 dig 3
	// 9 9 dig 2, 10 10 F, 11 11 A, 12 12 dig 1

	display.Begin(COMMON_ANODE, 3, 12, 48, A0, 0, 11, A3, A2, A4, A5, 10, A1, 38);
	display.SetBrightness(100);

	char testString[30];
	dtostrf(6.23, 3, 2, testString);
	DEBUG_PRINT(testString);
}

void loop() {

#ifndef DEBUG
	ModbusRTUServer.poll();
#endif

	unsigned long currentTime = millis();

	updateSolenoids(solenoids, currentTime);

	bool selectRisingEdge = checkForRisingEdge(select, digitalRead(SELECT));
	if (selectRisingEdge) {
		DEBUG_PRINT("select rising edge!");
	}
	bool valueDownRisingEdge = checkForRisingEdge(valueDown, digitalRead(VALUE_DOWN));
	bool valueUpRisingEdge = checkForRisingEdge(valueUp, digitalRead(VALUE_UP));
	updateInputHandler(inputHandler, selectRisingEdge, valueUpRisingEdge, valueDownRisingEdge);

	// the display
	if (inputHandler.editing) {
		if (currentTime % 1000 > 500) {
			display.DisplayString(inputHandler.displayString, inputHandler.decimalLocation);
		}
	} else {
		char outputString[3];
		sprintf(outputString, "%3d", solenoids.currentSolenoid);
		display.DisplayString(outputString, 0);
	}
	
	// Manage EEPROM
	if ((currentTime - lastEEPROMWrite > 10000) || (currentTime < lastEEPROMWrite)) {
		persistentVals.operationTime = elapsedTime(operationTimer, millis());
		persistentVals.lifeTime = elapsedTime(lifetimeTimer, millis());
		DEBUG_PRINT("Updated lifetime (sec):");
		DEBUG_PRINT(persistentVals.lifeTime);

		lastEEPROMWrite = currentTime;
		// NOTE: needs to be changed, this is a simple solution that doesn't have good enough endurance (100,000 writes guaranteed on EEPROM)
		UserSettings us = modbusGetUserSettings();
		EEPROM.put(0x00, EEPROMWrapper {
			EEPROM_WRITTEN_CONST, persistentVals, us
		});
	}

} 