// Structs for storing information that needs to persist after shutdown
// NOTE: this is a naive implementation, we'll need to think about EEPROM endurance
// as it only has 100,000 writes guaranteed

// basic check that the eeprom was written by this program by specifying some number that indicates it has valid settings
const byte EEPROM_WRITTEN_CONST = 0xAB;

struct UserSettings {
    double highAlarmThreshold;		// 0.00 - 10.00 InWC
    int highAlarmDelay;				// 0 - 600 seconds
    int downtimeCleaningDuration;	// 0 - 30 minutes
    double lowLimit;				// 0.00 - 10.00 InWC
    double highLimit;   			// 0.00 - 10.00 InWC
    int pulseOnTime;				// 50 - 500 milliseconds
    int pulseOffTime;				// 1 - 60 seconds
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