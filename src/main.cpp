#include <Arduino.h>
#include <persistent.h>
#include <pin_defs.h>
#include <timer.h>
#include <ArduinoModbus.h>
#include <solenoid.h>
#include <debug.h>
#include <modbus_registers.h>
#include <input_handler.h>
#include <LiquidCrystal.h>
#include <Adafruit_ADS1X15.h>
#include <pressure.h>
#include <Ethernet2.h>
#include <SPI.h>
#include <SD.h>
#include <SoftwareSerial.h>

extern ModbusTCPServer modbusTCPServer;

// Global variables
PersistentVals persistentVals;
UserSettings userSettings;

Timer operationTimer;
Timer lifetimeTimer;
Timer highAlarmTimer;
Timer cleaningTimer;

const unsigned long PULSE_CYCLE_MAINTENANCE_THRESHOLD = 1000000;
unsigned long lastEEPROMWrite = 0;
unsigned long lastPressureRead = 0;
unsigned long lastDowntimeUpdate = 0;

Solenoids solenoids = newSolenoids();

InputHandler inputHandler = newInputHandler();
ButtonDebouncer select = newButtonDebouncer();
ButtonDebouncer valueDown = newButtonDebouncer();
ButtonDebouncer valueUp = newButtonDebouncer();
ButtonDebouncer highAlarmReset = newButtonDebouncer();
// Temporary way of changing between pressure, manual, and downtime cleaning modes, hooked up to manual_cleaning_input
ButtonDebouncer stateChanger = newButtonDebouncer();
ButtonDebouncer operationReset = newButtonDebouncer();

bool highAlarmOn = false;

// rs = 43
// r/w = 41
// E = 39
// db0 - db3 = 37, 35, 33, 31
// db4 - db7 = 29 27 25 23
LiquidCrystal lcd(43, 41, 39, 29, 27, 25, 23);

extern Adafruit_ADS1115 pressureReader;

EthernetServer ethServer(502);
byte mac[] = {
	0xA8, 0x61, 0x0A, 0xAE, 0x85, 0x0B
};
IPAddress ip(192, 168, 1, 177);

float currentPressure = 0.00;

EthernetClient client;

enum CleaningState {
	PressureMode,
	ManualMode,
	DowntimeMode,
} cleaningState;

const char* DATA_FILENAME = "data.txt";

// Pin used for SD card communication
// on the Ethernet Shield
const int SD_PIN = 4;

// Bluetooth related variables
SoftwareSerial bluetoothSerial(BLUETOOTH_TX, BLUETOOTH_RX);
char bluetoothMessage[100] = {'\0'};
int bluetoothMessagePosition = 0;

void setup() {
	Serial.begin(9600);
	while (!Serial) {
		;	// again, might not need
	}

	DEBUG_PRINT("Serial debugging enabled");

	Ethernet.begin(mac, ip);
	ethServer.begin();

	DEBUG_PRINT("server at");
	DEBUG_PRINT(Ethernet.localIP());

	// Initialize the SD Card and read the settings
	SettingsWrapper wrapper;
	if (!SD.begin(SD_PIN)) {
		DEBUG_PRINT("Failed to init SD card");
	} else {
		DEBUG_PRINT("SD card initialization successful");
		File dataFile = SD.open(DATA_FILENAME, FILE_READ);
		if (dataFile) {
			int readStatus = dataFile.read(&wrapper, sizeof(SettingsWrapper)/sizeof(uint8_t));
			dataFile.close();
		} else {
			wrapper.written = 0; // use default settings below
		}
	}

	if (wrapper.written != EEPROM_WRITTEN_CONST) {
		DEBUG_PRINT("using defaults");
		persistentVals = newPersistentVals();
		userSettings = defaultSettings();
	} else {
		DEBUG_PRINT("using current");
		persistentVals = wrapper.persistentVals;
		userSettings = wrapper.userSettings;
	}

	setupTimingInputs();
	setupTimingOutputs();

	modbusInit(userSettings);

	// Assume these start for now
	operationTimer = newTimer(persistentVals.operationTime);
	startTimer(operationTimer, millis());
	lifetimeTimer = newTimer(persistentVals.lifeTime);
	startTimer(lifetimeTimer, millis());
	DEBUG_PRINT("Total lifetime (seconds):");
	DEBUG_PRINT(lifetimeTimer.totalTime);

	highAlarmTimer = newTimer(0);
	cleaningTimer = newTimer(0);
	cleaningState = PressureMode;
	
	lcd.begin(16, 4);
	lcd.print("hello");

	if (!pressureReader.begin()) {
		DEBUG_PRINT("Failed to start the ADC.");
	}

	// bluetooth chip setup
	bluetoothSerial.begin(115200);
	bluetoothSerial.print("$");
	bluetoothSerial.print("$");
	bluetoothSerial.print("$");
	delay(100);
	bluetoothSerial.println("U,9600,N");
	bluetoothSerial.begin(9600);
}

void loop() {
	unsigned long currentTime = millis();

	// Handle TCP client
	if (client) {
		if (client.connected()) {
			modbusTCPServer.poll();
		} else {
			client.stop();
			client = ethServer.available();
			if (client) modbusTCPServer.accept(client);
		}
	} else {
		client = ethServer.available();
		if (client) modbusTCPServer.accept(client);
	}

	// update pressure reading
	if (cleaningState == PressureMode) {
		if ((currentTime - lastPressureRead > 100) || (currentTime < lastPressureRead)) {
			lastPressureRead = currentTime;
			currentPressure = readPressureSensor();
			writeModbusFloat(Pressure, currentPressure);
			if (!inputHandler.editing) {
				lcd.clear();
				lcd.print("Pressure");
				lcd.setCursor(0, 1);
				lcd.print(currentPressure);
			}
		}
	} else if (cleaningState == DowntimeMode) {
		int totalSeconds = 60*modbusTCPServer.holdingRegisterRead(DowntimeCleaningDuration);
		int downtimeCleaningDifference = (totalSeconds) - elapsedTime(cleaningTimer, currentTime);
		if ((currentTime - lastDowntimeUpdate > 1000) || (currentTime < lastDowntimeUpdate)) {
			lastDowntimeUpdate = currentTime;
			if (!inputHandler.editing) {
				lcd.clear();
				lcd.print("Downtime");
				lcd.setCursor(0, 1);
				lcd.print(downtimeCleaningDifference/60);
				lcd.print(":");
				int remainder = downtimeCleaningDifference % 60;
				if (remainder == 0) {
					lcd.print("00");
				} else if (remainder < 10) {
					lcd.print("0");
					lcd.print(remainder);
				} else {
					lcd.print(remainder);
				}
			}
		}
		if (downtimeCleaningDifference < 0) {
			cleaningState = PressureMode;
		}
	}

	// fire solenoids if necessary, TODO: allow manual override and downtime override
	updateSolenoids(solenoids, currentTime, currentPressure, !(cleaningState == PressureMode));

	// Operation and lifetime timers should not be running under 0.5 InWC
	if (currentPressure < 0.5) {
		if (lifetimeTimer.running) {
			stopTimer(lifetimeTimer, currentTime);
		}
		if (operationTimer.running) {
			stopTimer(operationTimer, currentTime);
		}
	} else {
		if (!lifetimeTimer.running) {
			startTimer(lifetimeTimer, currentTime);
		}
		if (!operationTimer.running) {
			startTimer(operationTimer, currentTime);
		}
	}

	// Handle high alarm timer
	bool highAlarmResetRisingEdge = checkForRisingEdge(highAlarmReset, digitalRead(HIGH_ALARM_RESET));
	if (highAlarmOn && highAlarmResetRisingEdge) {
		highAlarmOn = false;
		digitalWrite(HIGH_ALARM, LOW);
		highAlarmTimer = newTimer(0);
	}
	if (currentPressure > readModbusFloat(HighAlarm)) {
		if (highAlarmTimer.running) {
			if (!highAlarmOn) {
				if (elapsedTime(highAlarmTimer, currentTime) > modbusTCPServer.holdingRegisterRead(HighAlarmDelay)) {
					digitalWrite(HIGH_ALARM, HIGH);
					highAlarmOn = true;
				}
			}
		} else {
			startTimer(highAlarmTimer, currentTime);
		}
	} else {
		if (highAlarmTimer.running) {
			highAlarmTimer = newTimer(0);
		}
	}

	bool opResetRisingEdge = checkForRisingEdge(operationReset, digitalRead(OPERATION_RESET));
	if (opResetRisingEdge) {
		operationTimer = newTimer(0);
		startTimer(operationTimer, currentTime);
		modbusTCPServer.holdingRegisterWrite(OperationTimer, 0);
	}

	// update the input handler
	bool selectRisingEdge = checkForRisingEdge(select, digitalRead(SELECT));
	bool valueDownRisingEdge = checkForRisingEdge(valueDown, digitalRead(VALUE_DOWN));
	bool valueUpRisingEdge = checkForRisingEdge(valueUp, digitalRead(VALUE_UP));
	updateInputHandler(inputHandler, selectRisingEdge, valueUpRisingEdge, valueDownRisingEdge);
	if (selectRisingEdge || valueUpRisingEdge|| valueDownRisingEdge) {
		lcd.clear();
		if (inputHandler.editing) {
			lcd.print(regToString(inputHandler.currentRegister));
			lcd.setCursor(0, 1);
			if (registerIsFloat(inputHandler.currentRegister)) {
				lcd.print(inputHandler.currentFloat);
			} else {
				lcd.print(inputHandler.currentValue);
			}
		}
	}

	// Change states (TODO: temporary for testing)
	bool changeStateRisingEdge = checkForRisingEdge(stateChanger, digitalRead(MANUAL_OVERRIDE));
	if (changeStateRisingEdge && !inputHandler.editing) {
		if (cleaningState == PressureMode) {
			cleaningState = ManualMode;
			lcd.clear();
			lcd.print("Manual");
		} else if (cleaningState == ManualMode) {
			cleaningState = DowntimeMode;
			cleaningTimer = newTimer(0);
			startTimer(cleaningTimer, currentTime);
		} else {
			cleaningState = PressureMode;
		}
	}

	// Manage EEPROM
	if ((currentTime - lastEEPROMWrite > 10000) || (currentTime < lastEEPROMWrite)) {
		persistentVals.operationTime = elapsedTime(operationTimer, millis());
		persistentVals.lifeTime = elapsedTime(lifetimeTimer, millis());
		DEBUG_PRINT("Updated lifetime (sec):");
		DEBUG_PRINT(persistentVals.lifeTime);
		DEBUG_PRINT(persistentVals.operationTime);

		lastEEPROMWrite = currentTime;
		// NOTE: needs to be changed, this is a simple solution that doesn't have good enough endurance (100,000 writes guaranteed on EEPROM)
		UserSettings us = modbusGetUserSettings();

		SettingsWrapper newSettings = SettingsWrapper {
			EEPROM_WRITTEN_CONST, persistentVals, us
		};
		
		// remove the file first, TODO: maybe a better way?
		SD.remove((char*) DATA_FILENAME);

		File dataFile = SD.open(DATA_FILENAME, FILE_WRITE);
		if (dataFile) {
			dataFile.write((uint8_t*) &newSettings, sizeof(SettingsWrapper)/sizeof(uint8_t));
			dataFile.close();
		}

		modbusTCPServer.holdingRegisterWrite(OperationTimer, persistentVals.operationTime);
		modbusTCPServer.holdingRegisterWrite(LifetimeTimer, persistentVals.lifeTime);
	}

	// Handle Bluetooth
	if (bluetoothSerial.available()) {
		char charRead = bluetoothSerial.read();

		// if message ending process the message, otherwise add to the message buffer
		if (charRead == '\r' || charRead == '\n' || charRead == '\0') {
			char* token = strtok(bluetoothMessage, " ");

			// check for the following commands: mode, pressure, set
			if (strstr(token, "mode")) {
				char* response = "OK";
				// should be the mode that we want
				token = strtok(NULL, " ");
				if (strstr(token, "downtime")) {
					if (cleaningState != DowntimeMode) {
						cleaningState = DowntimeMode;
						cleaningTimer = newTimer(0);
						startTimer(cleaningTimer, currentTime);
					}
				} else if (strstr(token, "manual")) {
					if (cleaningState != ManualMode) {
						cleaningState = ManualMode;
						lcd.clear();
						lcd.print("Manual");
					}
				} else if (strstr(token, "pressure")) {
					if (cleaningState != PressureMode) {
						cleaningState = PressureMode;
					}
				} else {
					response = "Invalid mode";
				}
				bluetoothSerial.println(response);
			} else if (strstr(bluetoothMessage, "pressure")) {
				char buffer[5];
				dtostrf(currentPressure, 4, 2, buffer);
				bluetoothSerial.print(buffer);
				bluetoothSerial.println(" InWC");
			} else if (strstr(bluetoothMessage, "set")) {
				token = strtok(NULL, " ");
				ModbusRegisters reg = NULL;

				if (strstr(token, "ontime")) {
					reg = PulseOnTime;
				} else if (strstr(token, "offtime")) {
					reg = PulseOffTime;
				} else if (strstr(token, "numsolenoids")) {
					reg = NumSolenoids;
				} else if (strstr(token, "downtime")) {
					reg = DowntimeCleaningDuration;
				} else {
					bluetoothSerial.println("Invalid setting");
				}

				if (reg != NULL) {
					// advance twice to skip "to"
					token = strtok(NULL, " ");
					token = strtok(NULL, " ");
					int value = atoi(token);
					modbusTCPServer.holdingRegisterWrite(reg, value);
					bluetoothSerial.println("OK");
				}
			}

			// reset the string
			memset(bluetoothMessage, '\0', bluetoothMessagePosition);
			bluetoothMessagePosition = 0;
		} else {
			bluetoothMessage[bluetoothMessagePosition] = charRead;
			bluetoothMessagePosition++;
		}
	}
}