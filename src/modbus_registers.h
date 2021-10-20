#include <debug.h>
#include <persistent.h>
#include <ArduinoModbus.h>

#ifndef MODBUS_REGISTERS
#define MODBUS_REGISTERS

const int MODBUS_SLAVE_ADDRESS = 1; // we'll need to figure this out

union ModbusFloat {
    float F;
    uint16_t ui[2];
};

// NOTE: this will write this and the following address
// so each modbus float should have two registers reserved
void writeModbusFloat(int address, float n) {
    ModbusFloat mf;
    mf.F = n;
    ModbusRTUServer.holdingRegisterWrite(address, mf.ui[0]);
    ModbusRTUServer.holdingRegisterWrite(address + 1, mf.ui[1]);
}

float readModbusFloat(int address) {
    ModbusFloat mf;
    mf.ui[0] = ModbusRTUServer.holdingRegisterRead(address);
    mf.ui[1] = ModbusRTUServer.holdingRegisterRead(address + 1);
    return mf.F;
}

enum ModbusRegisters {
    // sensor output
    Pressure = 0,
    OperationTimer = 2,
    LifetimeTimer = 3,

    // user settings
    HighAlarm = 4,
    HighAlarmDelay = 6,
    DowntimeCleaningDuration = 7,
    LowLimit = 8,
    HighLimit = 10,
    PulseOnTime = 12,
    PulseOffTime = 13,
};

// for editing the user settings (TODO: this may not make sense)
/* order:
- pulse off
- pulse on
- high limit
- low limit
- high alarm
- low alarm
- cycle delay
- down time
- auto alarm?
*/
int nextRegister(int current = 0) {
    if (current == 0) {
        return PulseOffTime;
    } else if (current == PulseOffTime) {
        return PulseOnTime;
    } else if (current == PulseOnTime) {
        return HighLimit;
    } else {
        return 0;
    }
}

void modbusInit(UserSettings userSettings) {
#ifndef DEBUG
    ModbusRTUServer = ModbusRTUServerClass();

	if (!ModbusRTUServer.begin(MODBUS_SLAVE_ADDRESS, 115200)) {
		DEBUG_PRINT("Starting Modbus server failed...");
	} else {
        ModbusRTUServer.configureHoldingRegisters(0x00, 14);

        // Outputs
        writeModbusFloat(Pressure, 5.3);
        ModbusRTUServer.holdingRegisterWrite(OperationTimer, 5);
        ModbusRTUServer.holdingRegisterWrite(LifetimeTimer, 5);

        writeModbusFloat(HighAlarm, userSettings.highAlarmThreshold);
        ModbusRTUServer.holdingRegisterWrite(HighAlarmDelay, userSettings.highAlarmDelay);
        ModbusRTUServer.holdingRegisterWrite(DowntimeCleaningDuration, userSettings.downtimeCleaningDuration);
        writeModbusFloat(LowLimit, userSettings.lowLimit);
        writeModbusFloat(HighLimit, userSettings.highLimit);
        ModbusRTUServer.holdingRegisterWrite(PulseOnTime, userSettings.pulseOnTime);
        ModbusRTUServer.holdingRegisterWrite(PulseOffTime, userSettings.pulseOffTime);
	}
#endif
}

UserSettings modbusGetUserSettings() {
    // Note: castings to int16_t should never be an issue since reading a holding register should only return 16 bits. Not sure why the library returns a long instead
    return UserSettings {
        readModbusFloat(HighAlarm),
        (int16_t) ModbusRTUServer.holdingRegisterRead(HighAlarmDelay),
        (int16_t) ModbusRTUServer.holdingRegisterRead(DowntimeCleaningDuration),
        readModbusFloat(LowLimit),
        readModbusFloat(HighLimit),
        (int16_t) ModbusRTUServer.holdingRegisterRead(PulseOnTime),
        (int16_t) ModbusRTUServer.holdingRegisterRead(PulseOffTime),
    };
}

#endif
