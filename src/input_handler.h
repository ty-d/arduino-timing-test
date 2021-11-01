#ifndef INCLUDE_INPUT_HANDLER
#define INCLUDE_INPUT_HANDLER

#include <pin_defs.h>
#include <ArduinoModbus.h>
#include <modbus_registers.h>

// a generic button debouncer that gives the state of the button
struct ButtonDebouncer {
    bool wasPressed;
    unsigned int numTimesHigh;
};

ButtonDebouncer newButtonDebouncer();

bool checkForRisingEdge(ButtonDebouncer& bd, int reading);

char* regToString(int reg);

struct InputHandler {
    int currentRegister;
    int currentValue;
    float currentFloat;
    bool editing;
    int decimalLocation;
};

InputHandler newInputHandler();

bool registerIsFloat(int reg);

float getIncrFloat(int reg);

int getIncrInt(int reg);

int getUpperInt(int reg);

int getLowerInt(int reg);

float getUpperFloat(int reg);

float getLowerFloat(int reg);

// Returns -1 if it should not be displaying anything, the number to display otherwise
void updateInputHandler(InputHandler& ih, bool select, bool valueUp, bool valueDown);

#endif