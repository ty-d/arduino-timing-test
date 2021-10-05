// Structs for storing information that needs to persist after shutdown
// NOTE: this is a naive implementation, we'll need to think about EEPROM endurance
// as it only has 100,000 writes guaranteed

// basic check that the eeprom was written by this program by specifying some number that indicates it has valid settings
#ifndef PERSISTENT_INCLUDED
#define PERSISTENT_INCLUDED

const byte EEPROM_WRITTEN_CONST = 0xAC;

// Allows easy splitting of float into two integers for modbus writes
struct UserSettings {
    float highAlarmThreshold;		// 0.00 - 10.00 InWC
    int16_t highAlarmDelay;				// 0 - 600 seconds
    int16_t downtimeCleaningDuration;	// 0 - 30 minutes
    float lowLimit;				// 0.00 - 10.00 InWC
    float highLimit;   			// 0.00 - 10.00 InWC
    int16_t pulseOnTime;				// 50 - 500 milliseconds
    int16_t pulseOffTime;				// 1 - 60 seconds
};

// TODO: confirm these
UserSettings defaultSettings() {
    return UserSettings {
        6.0, 5, 5, 0.00, 6.00, 150, 10
    };
}

// Contains all values that should persist between runs
struct PersistentVals {
    unsigned long operationTime;    // seconds
    unsigned long lifeTime;         // seconds
    unsigned long pulseCycleCount;  // for maintenance alarm
};

PersistentVals newPersistentVals() {
    return PersistentVals {
        0, 0, 0
    };
}

struct EEPROMWrapper {
    byte written; // signal that we wrote the EEPROM, there might be something better
    PersistentVals persistentVals;
    UserSettings userSettings;
};

#endif