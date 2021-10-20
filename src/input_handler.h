#include <pin_defs.h>
#include <ArduinoModbus.h>
#include <modbus_registers.h>

#ifndef INCLUDE_INPUT_HANDLER
#define INCLUDE_INPUT_HANDLER

const int DEBOUNCE_DELAY = 50;

// a generic button debouncer that gives the state of the button
struct ButtonDebouncer {
    bool wasPressed;
    unsigned int numTimesHigh;
};

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

    return (bd.numTimesHigh == 5);
}

struct InputHandler {
    int currentRegister;
    int currentValue;
    float currentFloat;
    bool editing;
    char displayString[3];
    int decimalLocation;
};

InputHandler newInputHandler() {
    return InputHandler {
        0, 0, false, "000", 0
    };
}

// Returns -1 if it should not be displaying anything, the number to display otherwise
void updateInputHandler(InputHandler& ih, bool select, bool valueUp, bool valueDown) {
    if (select) {
        if (ih.editing) {
            ModbusRTUServer.holdingRegisterWrite(ih.currentRegister, ih.currentValue);
            ih.currentRegister = nextRegister(ih.currentRegister);
            ih.currentValue = ModbusRTUServer.holdingRegisterRead(ih.currentRegister);
            if (ih.currentRegister == 0) {
                ih.editing = false;
            }
            if (ih.currentRegister == HighLimit) {
                ih.currentFloat = readModbusFloat(ih.currentRegister);
            }
        } else {
            ih.editing = true;
            ih.currentRegister = nextRegister();
            ih.currentValue = ModbusRTUServer.holdingRegisterRead(ih.currentRegister);
            if (ih.currentRegister == HighLimit) {
                DEBUG_PRINT("writing float");
                ih.currentFloat = readModbusFloat(ih.currentRegister);
            }
        }
    } else if ((valueUp || valueDown) && ih.editing) {
        if (ih.currentRegister != HighLimit) {
            if (valueUp) {
                ih.currentValue++;
            } else {
                ih.currentValue--;
            }
        } else {
            if (valueUp) {
                ih.currentFloat += .01;
            } else {
                ih.currentFloat -= .01;
            }
        }
    }

    if (ih.editing) {
        if (ih.currentRegister == HighLimit) {
			char tempString[4];
			dtostrf(ih.currentFloat, 4, 2, tempString);
			ih.displayString[0] = tempString[0];
			ih.displayString[1] = tempString[2];
			ih.displayString[2] = tempString[3];
            ih.decimalLocation = 1;
        } else {
            sprintf(ih.displayString, "%3d", ih.currentValue);
            ih.decimalLocation = 0;
        }
    }
}

#endif