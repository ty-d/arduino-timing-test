#include <Arduino.h>
#include <persistent.h>
#include <pin_defs.h>
#include <EEPROM.h>
#include <timer.h>

// If defined will use the serial monitor to output debug info
#define DEBUG

#ifdef DEBUG
#define DEBUG_PRINT(arg)    Serial.println(arg)
#else
#define DEBUG_PRINT(arg)    // Don't do anything in release builds
#endif

// Global variables
PersistentVals persistentVals;
UserSettings userSettings;

Timer operationTimer;
Timer lifetimeTimer;
Timer highAlarmTimer;
Timer cleaningTimer;

const int PULSE_CYCLE_MAINTENANCE_THRESHOLD = 1000; // What should this value be? is it constant?
unsigned long lastEEPROMWrite = 0;
unsigned int currentSolenoid = 0;

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
	unsigned long currentTime = millis();
	if ((currentTime - lastEEPROMWrite > 10000) || (currentTime < lastEEPROMWrite)) {
		persistentVals.operationTime = elapsedTime(operationTimer, millis());
		persistentVals.lifeTime = elapsedTime(lifetimeTimer, millis());
		DEBUG_PRINT("Updated lifetime (sec):");
		DEBUG_PRINT(persistentVals.lifeTime);

		lastEEPROMWrite = currentTime;
		// NOTE: needs to be changed, this is a simple solution that doesn't have good enough endurance (100,000 writes guaranteed on EEPROM)
		EEPROM.put(0x00, EEPROMWrapper {
			EEPROM_WRITTEN_CONST, persistentVals, userSettings
		});
		DEBUG_PRINT(lastEEPROMWrite);
	}

	// assume a pulse for now
	digitalWrite(SOLENOID_ARRAY[currentSolenoid], HIGH);
	delay(userSettings.pulseOnTime);
	digitalWrite(SOLENOID_ARRAY[currentSolenoid], LOW);
	delay(userSettings.pulseOffTime * 1000);

	currentSolenoid = (currentSolenoid + 1) % NUM_SOLENOIDS;
}