#include <modbus_registers.h>

ModbusTCPServer modbusTCPServer;

void writeModbusFloat(int address, float n) {
    ModbusFloat mf;
    mf.F = n;
    modbusTCPServer.holdingRegisterWrite(address, mf.ui[0]);
    modbusTCPServer.holdingRegisterWrite(address + 1, mf.ui[1]);
}

float readModbusFloat(int address) {
    ModbusFloat mf;
    mf.ui[0] = modbusTCPServer.holdingRegisterRead(address);
    mf.ui[1] = modbusTCPServer.holdingRegisterRead(address + 1);
    return mf.F;
}

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
    } else if (current == HighLimit) {
        return LowLimit;
    } else if (current == LowLimit) {
        return HighAlarm;
    } else if (current == HighAlarm) {
        return HighAlarmDelay;
    } else if (current == HighAlarmDelay) {
        return DowntimeCleaningDuration;
    } else if (current == DowntimeCleaningDuration) {
        return NumSolenoids;
    } else {
        return 0;
    }
}

void modbusInit(UserSettings userSettings) {
    modbusTCPServer = ModbusTCPServer();
	if (!modbusTCPServer.begin()) {
		DEBUG_PRINT("Starting Modbus server failed...");
	} else {
        modbusTCPServer.configureHoldingRegisters(0x00, 15);

        // Outputs
        writeModbusFloat(Pressure, 5.3);
        modbusTCPServer.holdingRegisterWrite(OperationTimer, 5);
        modbusTCPServer.holdingRegisterWrite(LifetimeTimer, 5);

        writeModbusFloat(HighAlarm, userSettings.highAlarmThreshold);
        modbusTCPServer.holdingRegisterWrite(HighAlarmDelay, userSettings.highAlarmDelay);
        modbusTCPServer.holdingRegisterWrite(DowntimeCleaningDuration, userSettings.downtimeCleaningDuration);
        writeModbusFloat(LowLimit, userSettings.lowLimit);
        writeModbusFloat(HighLimit, userSettings.highLimit);
        modbusTCPServer.holdingRegisterWrite(PulseOnTime, userSettings.pulseOnTime);
        modbusTCPServer.holdingRegisterWrite(PulseOffTime, userSettings.pulseOffTime);
        modbusTCPServer.holdingRegisterWrite(NumSolenoids, userSettings.numSolenoids);
	}
}

UserSettings modbusGetUserSettings() {
    // Note: castings to int16_t should never be an issue since reading a holding register should only return 16 bits. Not sure why the library returns a long instead
    return UserSettings {
        readModbusFloat(HighAlarm),
        (int16_t) modbusTCPServer.holdingRegisterRead(HighAlarmDelay),
        (int16_t) modbusTCPServer.holdingRegisterRead(DowntimeCleaningDuration),
        readModbusFloat(LowLimit),
        readModbusFloat(HighLimit),
        (int16_t) modbusTCPServer.holdingRegisterRead(PulseOnTime),
        (int16_t) modbusTCPServer.holdingRegisterRead(PulseOffTime),
        (int16_t) modbusTCPServer.holdingRegisterRead(NumSolenoids),
    };
}