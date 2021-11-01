#include <persistent.h>

// TODO: confirm these
UserSettings defaultSettings() {
    return UserSettings {
        6.0, 5, 5, 0.00, 6.00, 150, 10, 6
    };
}

PersistentVals newPersistentVals() {
    return PersistentVals {
        0, 0, 0
    };
}