#include <input_handler.h>

//extern ModbusTCPServer modbusTCPServer;

ButtonDebouncer newButtonDebouncer() {
    return ButtonDebouncer {
        false, 0,
    };
}

bool checkForRisingEdge(ButtonDebouncer& bd, int reading) {
    bool pressed = (reading == HIGH);
    if (bd.wasPressed && pressed) {
        bd.numTimesHigh++;
    } else if (!bd.wasPressed && pressed) {
        bd.numTimesHigh = 1;
    }

    bd.wasPressed = pressed;

    if (bd.numTimesHigh == 10) {
        bd.numTimesHigh++; // avoid being stuck at 10
        return true;
    }

    return false;
}

char* regToString(int reg) {
    if (reg == Pressure) {
        return "Pressure";
    } else if (reg == HighAlarm) {
        return "High Alarm";
    } else if (reg == HighAlarmDelay) {
        return "High Delay";
    } else if (reg == DowntimeCleaningDuration) {
        return "Downtime";
    } else if (reg == LowLimit) {
        return "Low Limit";
    } else if (reg == HighLimit) {
        return "High Limit";
    } else if (reg == PulseOnTime) {
        return "On Time";
    } else if (reg == PulseOffTime) {
        return "Off Time";
    } else if (reg == NumSolenoids) {
        return "Solenoids";
    } else {
        return "Unimplemented";
    }
}

InputHandler newInputHandler() {
    return InputHandler {
        0, 0, 0.0, false, 0
    };
}

bool registerIsFloat(int reg) {
    return (reg == HighLimit || reg == LowLimit || reg == HighAlarm);
}

float getIncrFloat(int reg) {
    return 0.01;
}

int getIncrInt(int reg) {
    if (reg == PulseOnTime) {
        return 10;
    }
    return 1;
}

int getUpperInt(int reg) {
    if (reg == HighAlarmDelay) {
        return 600;
    } else if (reg == DowntimeCleaningDuration) {
        return 30;
    } else if (reg == PulseOnTime) {
        return 500;
    } else if (reg == PulseOffTime) {
        return 60;
    } else if (reg == NumSolenoids) {
        return MAX_NUM_SOLENOIDS;
    }
}

int getLowerInt(int reg) {
    if (reg == PulseOnTime) {
        return 50;
    } else if ((reg == PulseOffTime) || (reg == NumSolenoids)) {
        return 1;
    } else {
        return 0;
    }
}

float getUpperFloat(int reg) {
    return 10.00;
}

float getLowerFloat(int reg) {
    return 0.00;
}

// Returns -1 if it should not be displaying anything, the number to display otherwise
void updateInputHandler(InputHandler& ih, bool select, bool valueUp, bool valueDown) {
    if (select) {
        if (ih.editing) {
            if (registerIsFloat(ih.currentRegister)) {
                writeModbusFloat(ih.currentRegister, ih.currentFloat);
            } else {
                modbusTCPServer.holdingRegisterWrite(ih.currentRegister, ih.currentValue);
            }
            ih.currentRegister = nextRegister(ih.currentRegister);
            ih.currentValue = modbusTCPServer.holdingRegisterRead(ih.currentRegister);
            if (ih.currentRegister == 0) {
                ih.editing = false;
            }
            if (registerIsFloat(ih.currentRegister)) {
                ih.currentFloat = readModbusFloat(ih.currentRegister);
            }
        } else {
            ih.editing = true;
            ih.currentRegister = nextRegister();
            ih.currentValue = modbusTCPServer.holdingRegisterRead(ih.currentRegister);
            if (registerIsFloat(ih.currentRegister)) {
                DEBUG_PRINT("writing float");
                ih.currentFloat = readModbusFloat(ih.currentRegister);
            }
        }
    } else if ((valueUp || valueDown) && ih.editing) {
        if (!registerIsFloat(ih.currentRegister)) {
            if (valueUp) {
                if (ih.currentValue < getUpperInt(ih.currentRegister)) {
                    ih.currentValue += getIncrInt(ih.currentRegister);
                }
            } else {
                if (ih.currentValue > getLowerInt(ih.currentRegister)) {
                    ih.currentValue -= getIncrInt(ih.currentRegister);
                }
            }
            modbusTCPServer.holdingRegisterWrite(ih.currentRegister, ih.currentValue);
        } else {
            if (valueUp) {
                if (ih.currentFloat < getUpperFloat(ih.currentRegister)) {
                    ih.currentFloat += getIncrFloat(ih.currentRegister);
                }
            } else {
                if (ih.currentFloat > getLowerFloat(ih.currentRegister)) {
                    ih.currentFloat -= getIncrFloat(ih.currentRegister);
                }
            }
            writeModbusFloat(ih.currentRegister, ih.currentFloat);
        }
    }
}
