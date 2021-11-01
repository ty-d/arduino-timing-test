#ifndef MODBUS_REGISTERS
#define MODBUS_REGISTERS

#include <debug.h>
#include <persistent.h>
#include <ArduinoModbus.h>

extern ModbusTCPServer modbusTCPServer;

const int MODBUS_SLAVE_ADDRESS = 1; // we'll need to figure this out

union ModbusFloat {
    float F;
    uint16_t ui[2];
};

// NOTE: this will write this and the following address
// so each modbus float should have two registers reserved
void writeModbusFloat(int address, float n);

float readModbusFloat(int address);

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
    NumSolenoids = 14,
};

int nextRegister(int current = 0);

void modbusInit(UserSettings userSettings);

UserSettings modbusGetUserSettings();

#endif
