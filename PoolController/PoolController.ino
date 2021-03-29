#include <SPI.h>
#include <WiFiNINA.h>
#include <OneWire.h>
#include <DallasTemperature.h>
#include <Keypad.h>
#include <Wire.h>
#include <DHT.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include "Firebase_Arduino_WiFiNINA.h"
#include "Firebase_Arduino_WiFiNINA_HTTPCLient.h"

FirebaseData firebaseData;

struct Data // struct to contain a copy of the database values
{
	bool Heater_Running = false;    // Relay 1
	bool Pump_Running = false;       // Relay 2
	float AirTemp = 10000;          // Air Temp Probe
	float WaterTemp = 10000;        // Water Temp Probe
	float pH = 10000;               // pH Temp Probe
	bool WaterLevel = false;        // Water Level Probe
};
Data data;

// Current Sensor Values - initialized to garbage values
bool Heater_Running = false;
bool Pump_Running = false;
float AirTemp = 10000;
float WaterTemp = 10000;
float pH = 10000;
bool WaterLevel = false;

const byte ROWS = 4;
const byte COLS = 4;

char hexaKeys[ROWS][COLS] = {
  {'1', '2', '3', 'A'},
  {'4', '5', '6', 'B'},
  {'7', '8', '9', 'C'},
  {'*', '0', '#', 'D'}
};
byte rowPins[COLS] = { 3, 4, 5, 6 };
byte colPins[ROWS] = { 7, 8, 9, 10 };

bool change = false;

Keypad customKeypad = Keypad(makeKeymap(hexaKeys), rowPins, colPins, ROWS, COLS);

// PIN DEFINITIONS
#define WATER_SENS_PROBE_DATA 2
#define AIR_TEMP_PROBE_DATA 12
#define DHTTYPE DHT22   // DHT22
#define KEYPAD_R1 3
#define KEYPAD_R2 4
#define KEYPAD_R3 5
#define KEYPAD_R4 6
#define KEYPAD_C1 7
#define KEYPAD_C2 8
#define KEYPAD_C3 9
#define KEYPAD_C4 10

#define USE_JOSIAH_HOTSPOT 0 
#define USE_LUKE_HOTSPOT 0 
#define USE_OCguest_NETWORK 1
#if USE_JOSIAH_HOTSPOT
#define WIFI_SSID "Josiah's S20+"
#define WIFI_PASS "7205858843"
#endif
#if USE_LUKE_HOTSPOT
#define WIFI_SSID "Luke's iPhone"
#define WIFI_PASS "shamballa"
#endif
#if USE_OCguest_NETWORK
#define WIFI_SSID "OCguest"
#define WIFI_PASS ""
#endif

#define FIREBASE_HOST "es-pool-controller-default-rtdb.firebaseio.com"
#define FIREBASE_AUTH "YXsdNJ9OFISN2ZwgUhIQxny6KDtDgdMdm9Ho6HWL"
#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 64 // OLED display height, in pixels
#define SCREEN_ADDRESS 0x3C 
#define OLED_RESET -1 // Reset pin # (-1 since sharing with Arduino Reset)

#define TIMEZONEOFFSET -5

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

int Fall_Back = 0;
int hrs;
int mins;
int secs;
int timeZoneOffset = TIMEZONEOFFSET * 60 * 60; // time zone offset relative to GMT 

void getTimeFromWifi() {
	// Get the time
	int time = WiFi.getTime() + timeZoneOffset;
	secs = time % 60; // Get seconds
	time /= 60;
	mins = time % 60; // Get minutes
	time /= 60;
	hrs = (time % 24) - Fall_Back; // Get hours
	if (hrs < 0 || hrs > 24) {
		hrs = 0;
		mins = 0;
		secs = 0;
	}
}

String getTimeString() { // Calls the getTime function and manipulates the values to create a string in a nice format
	getTimeFromWifi();
	bool AMFlag = true;
	String time = "Time: ";
	// Print time
	if (hrs > 12) {
		hrs -= 12;
		AMFlag = false;
	}
	time = time + hrs;
	time = time + ':';
	if (mins < 10) {
		time = time + '0';
	}
	time = time + mins;
	if (AMFlag) {
		time = time + " AM";
	}
	else {
		time = time + " PM";
	}
	return time;
}

// Initialization stuff
OneWire WaterTempSense(WATER_SENS_PROBE_DATA);
DallasTemperature WaterTempSensor(&WaterTempSense);
DHT AirTempSensor(AIR_TEMP_PROBE_DATA, DHTTYPE);

// functions to turn on or off the builtin LED
void LEDOn() {
	digitalWrite(LED_BUILTIN, HIGH);
}

void LEDOff() {
	digitalWrite(LED_BUILTIN, LOW);
}

// Attempts a wifi connection based upon the contents of ssid and pass
int wifiStatus = WL_IDLE_STATUS;
void SetupWifiConnection() {
	LEDOff();
	if (WIFI_PASS == "") {  // If password field is empty, attempt login without pass
		wifiStatus = WiFi.begin(WIFI_SSID);
	}
	else {
		wifiStatus = WiFi.begin(WIFI_SSID, WIFI_PASS); // Attempt login with pass
	}
}

bool GetFirebaseValues() {
	bool tempHeater;
	bool tempPump;
	// Pull Heater
	if (Firebase.getBool(firebaseData, "/Devices/Heater")) {
		tempHeater = firebaseData.boolData();
	}

	// Pull Filter / Pump
	if (Firebase.getBool(firebaseData, "/Devices/Pump")) {
		tempPump = firebaseData.boolData();
	}

	if (tempPump != data.Pump_Running || tempHeater != data.Heater_Running) {
		Pump_Running = tempPump;
		Heater_Running = tempHeater;
		return true; // returns if a change was detected
	}
	return false;
}

// Pushes values to update firebase
void UpdateFirebase(bool ManualUpdate) {
	// Set values
	if (ManualUpdate) {
		data.Heater_Running = Heater_Running;
		data.Pump_Running = Pump_Running;
	}
	data.WaterTemp = WaterTemp;
	data.AirTemp = AirTemp;
	data.pH = pH;
	WaterLevel = data.WaterLevel;

	Serial.println("========================================");
	// Push Heater
	if (Firebase.setBool(firebaseData, "/Devices/Heater", data.Heater_Running)) {
		Serial.println(firebaseData.dataPath() + " = " + data.Heater_Running);
	}
	else {
		Serial.println("Error with: Heater");
		Serial.println("Error: " + firebaseData.errorReason());
	}

	// Push Filter / Pump
	if (Firebase.setBool(firebaseData, "/Devices/Pump", data.Pump_Running)) {
		Serial.println(firebaseData.dataPath() + " = " + data.Pump_Running);
	}
	else {
		Serial.println("Error with: Pump / Filter");
		Serial.println("Error: " + firebaseData.errorReason());
	}

	// Push Water Temp
	if (Firebase.setFloat(firebaseData, "/Sensors/WaterTemp", data.WaterTemp)) {
		Serial.println(firebaseData.dataPath() + " = " + data.WaterTemp);
	}
	else {
		Serial.println("Error with: Water Temp");
		Serial.println("Error: " + firebaseData.errorReason());
	}

	// Push Air Temp
	if (Firebase.setFloat(firebaseData, "/Sensors/AirTemp", data.AirTemp)) {
		Serial.println(firebaseData.dataPath() + " = " + data.AirTemp);
	}
	else {
		Serial.println("Error with: Air Temp");
		Serial.println("Error: " + firebaseData.errorReason());
	}

	// Push pH
	if (Firebase.setFloat(firebaseData, "/Sensors/pH", data.pH)) {
		Serial.println(firebaseData.dataPath() + " = " + data.pH);
	}
	else {
		Serial.println("Error with: pH");
		Serial.println("Error: " + firebaseData.errorReason());
	}

	// Push Water Level
	if (Firebase.setBool(firebaseData, "/Sensors/WaterLevel", data.WaterLevel)) {
		Serial.println(firebaseData.dataPath() + " = " + data.WaterLevel);
	}
	else {
		Serial.println("Error with: Water Level");
		Serial.println("Error: " + firebaseData.errorReason());
	}
	Serial.println("=======================================");
}

void UpdateRelays() {
	// This function will set pins HIGH or LOW to turn relays on or off

}

void ReadSensors() {
	// Air Temp
	AirTemp = AirTempSensor.readTemperature();
	AirTemp = (AirTemp * 1.8) + 32; // Convert to F

	// Water Temp
	WaterTempSensor.requestTemperatures();
	delay(25);
	WaterTemp = WaterTempSensor.getTempFByIndex(0);

	// pH


	// Water Level


}

// Keypad stuff
void keypadEvent(KeypadEvent key) {
	switch (customKeypad.getState()) {
	case PRESSED:
		if (key == '*') {
			Serial.println("pressed...");

			if (change == false) {
				change = true;
			}
			else {
				change = false;
			}
		}
		break;
	}
}

// More keypad stuff
void getnum() {
	char key = customKeypad.waitForKey();
	while (change == true) {
		if (key != '#') {
			Serial.print(key);
		}
		else {
			Serial.println();
		}

		key = customKeypad.waitForKey();
	}
}


////////////////////// START THE MACHINE
void setup() {

	pinMode(LED_BUILTIN, OUTPUT);

	customKeypad.addEventListener(keypadEvent);

	WaterTempSensor.begin();
	AirTempSensor.begin();

	while (!Serial) {
		; // wait for serial connection
	}

	Wire.begin();

	// -------------------------------------------------------------
	// Setup display

	display.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS);
	display.display();
	display.clearDisplay();
	display.clearDisplay();
	display.display();
	display.drawPixel(0, 0, WHITE);
	display.drawPixel(127, 0, WHITE);
	display.drawPixel(0, 63, WHITE);
	display.drawPixel(127, 63, WHITE);
	display.setTextSize(1);
	display.setTextColor(WHITE);
	display.setCursor(40, 25);
	display.print("ONLINE");
	display.display();
	delay(1000);

	// -------------------------------------------------------------

	// -------------------------------------------------------------
	// Start Wifi & Connect to Firebase

	SetupWifiConnection();

	while (wifiStatus != WL_CONNECTED) {
		Serial.println("Wifi Connection Failed... retrying...");
		SetupWifiConnection();
		delay(10000); // Delay for 10 seconds
	}

	LEDOn();
	Serial.println();
	Serial.print("Connected to ");
	Serial.print(WIFI_SSID);
	Serial.print(" as: ");
	Serial.println(WiFi.localIP());

	Firebase.begin(FIREBASE_HOST, FIREBASE_AUTH, WIFI_SSID, WIFI_PASS); // Connect to firebase
	Firebase.reconnectWiFi(true);

	// Wifi & Firebase should be connected
	// -------------------------------------------------------------

	// -------------------------------------------------------------
	// Setup Pins regarding any pull-up nature or pull-down nature they require

}

int Update_Firebase_Counter = 30;
void loop() {

	char key = customKeypad.getKey();
	if (change == true) getnum();

	// -------------------------------------------------------------
	// Write Data to Display

	String Pump_Running_String, Heater_Running_String;
	if (Pump_Running == false) {
		Pump_Running_String = "OFF";
	}
	else {
		Pump_Running_String = "ON";
	}
	if (Heater_Running == false) {
		Heater_Running_String = "OFF";
	}
	else {
		Heater_Running_String = "ON";
	}

	display.clearDisplay();
	display.setTextSize(1);
	display.setCursor(1, 1);
	display.println(getTimeString());
	display.println("Sensors:");
	display.print("Air Tem: "); display.println(AirTemp);
	display.print("Water Temp: "); display.println(WaterTemp);
	display.print("pH: "); display.println(pH);
	display.println("Relays:");
	display.print("Pump: "); display.println(Pump_Running_String);
	display.print("Heater: "); display.println(Heater_Running_String);
	display.display();

	// -------------------------------------------------------------

	// -------------------------------------------------------------
	// Read sensor and firebase data and store into memory

	bool FirebaseChangeMade = GetFirebaseValues();
	ReadSensors(); // Read sensors and store their values

	// Data read in by this point
	// -------------------------------------------------------------

	// -------------------------------------------------------------
	// Make appropriate relay modifications

	UpdateRelays(); // Turn Relays on or off based on the data struct's values

	// -------------------------------------------------------------

	// -------------------------------------------------------------
	// Upload updated data to firebase

	if (Update_Firebase_Counter > 29 || FirebaseChangeMade) // If Update_Firebase_Counter gets to 30, push values to firebase (about 30 seconds)
	{
		UpdateFirebase(FirebaseChangeMade); // Push updated values to Firebase
		Update_Firebase_Counter = 0;
	}

	// -------------------------------------------------------------
	delay(500);
	Update_Firebase_Counter++;
}
