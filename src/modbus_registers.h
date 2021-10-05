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
        ModbusRTUServer.holdingRegisterWrite(HighAlarmDelay, 4);
        ModbusRTUServer.holdingRegisterWrite(DowntimeCleaningDuration, 8);
        writeModbusFloat(LowLimit, 0.01);
        writeModbusFloat(HighLimit, 6.00);
        ModbusRTUServer.holdingRegisterWrite(PulseOnTime, 150);
        ModbusRTUServer.holdingRegisterWrite(PulseOffTime, 10);
	}
#endif
}

UserSettings modbusGetUserSettings() {
    return UserSettings {
        readModbusFloat(HighAlarm),
        ModbusRTUServer.holdingRegisterRead(HighAlarmDelay),
        ModbusRTUServer.holdingRegisterRead(DowntimeCleaningDuration),
        readModbusFloat(LowLimit),
        readModbusFloat(HighLimit),
        ModbusRTUServer.holdingRegisterRead(PulseOnTime),
        ModbusRTUServer.holdingRegisterRead(PulseOffTime),
    };
}

#endif
