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

struct Data // struct to contain a copy of the firebase values
{
	bool Mode = true;               // Pool Operates in auto or manual mode
	bool Heater_Running = false;    // Relay 1
	bool Pump_Running = false;      // Relay 2
	float AirTemp = 10000;          // Air Temp Probe
	float WaterTemp = 10000;        // Water Temp Probe
	float pH = 10000;               // pH Temp Probe
	bool WaterLevel = false;        // Water Level Probe
	String Schedule[7] = { "" };      // Array of strings holding the schedules for each day of the week
};
Data localFireData;

// Current Sensor Values - initialized to throw away values
static bool Mode = true;
static bool Heater_Running = false;
static bool Pump_Running = false;
static float AirTemp = 10000;
static float WaterTemp = 10000;
static float pH = 0.00;
static bool WaterLevel = false;

int Update_Firebase_Counter = 30;
bool ChangeMade;
bool FirebaseChangeMade;
bool LocalChangeMade;

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

bool keypadEditMode = false;

Keypad customKeypad = Keypad(makeKeymap(hexaKeys), rowPins, colPins, ROWS, COLS);

// PIN DEFINITIONS
#define WATER_SENS_PROBE_DATA 2   // Temp
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
#define pHSensorPin 17
#define pHOFFSET -5.00
#define HEATERPIN 20
#define PUMPPIN 21
#define WATERLVL 16

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
#define BLUE WHITE

#define TIMEZONEOFFSET -5

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

int Fall_Back = 0;
int hrs;
int mins;
int secs;
int timeZoneOffset = TIMEZONEOFFSET * 60 * 60; // time zone offset relative to GMT 

String getDay() {
	int time = WiFi.getTime() + timeZoneOffset;
	time = (time / 86400) % 7;

	switch (time) {
	case 0:
		return "Thursday";
		break;
	case 1:
		return "Friday";
		break;
	case 2:
		return "Saturday";
		break;
	case 3:
		return "Sunday";
		break;
	case 4:
return "Monday";
break;
	case 5:
		return "Tuesday";
		break;
	case 6:
		return "Wednesday";
		break;
	}
}
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
	int tempHrs = hrs, tempMins = mins;
	String time = "Time: ";
	// Print time
	if (tempHrs > 12) {
		tempHrs -= 12;
		AMFlag = false;
	}
	time = time + tempHrs;
	time = time + ':';
	if (tempMins < 10) {
		time = time + '0';
	}
	time = time + tempMins;
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

bool CheckSchedule(String Schedule) {
	getTimeFromWifi();
	int tempLHrs = (String(Schedule[0]) + String(Schedule[1])).toInt();
	int tempHHrs = (String(Schedule[6]) + String(Schedule[7])).toInt();
	int tempLMins = (String(Schedule[3]) + String(Schedule[4])).toInt();
	int tempHMins = (String(Schedule[9]) + String(Schedule[10])).toInt();
	if (hrs <= tempHHrs && hrs >= tempLHrs && mins <= tempHMins && mins >= tempLMins) {
		return true;
	}
	else {
		return false;
	}
}

bool GetFirebaseValues() {
	bool tempHeater;
	bool tempPump;
	bool tempMode;
	String tempSchedule[7];
	// Pull Heater
	if (Firebase.getBool(firebaseData, "/Devices/Heater")) {
		tempHeater = firebaseData.boolData();
	}
	else {
		Serial.println("Error: " + firebaseData.errorReason());
	}

	// Pull Filter / Pump
	if (Firebase.getBool(firebaseData, "/Devices/Pump")) {
		tempPump = firebaseData.boolData();
	}
	else {
		Serial.println("Error: " + firebaseData.errorReason());
	}

	// Mode
	if (Firebase.getBool(firebaseData, "/Auto")) {
		tempMode = firebaseData.boolData();
	}

	// Schedule
	if (Firebase.getString(firebaseData, "/Schedule/Monday")) {
		tempSchedule[0] = firebaseData.stringData();
	}
	if (Firebase.getString(firebaseData, "/Schedule/Tuesday")) {
		tempSchedule[1] = firebaseData.stringData();
	}
	if (Firebase.getString(firebaseData, "/Schedule/Wednesday")) {
		tempSchedule[2] = firebaseData.stringData();
	}
	if (Firebase.getString(firebaseData, "/Schedule/Thursday")) {
		tempSchedule[3] = firebaseData.stringData();
	}
	if (Firebase.getString(firebaseData, "/Schedule/Friday")) {
		tempSchedule[4] = firebaseData.stringData();
	}
	if (Firebase.getString(firebaseData, "/Schedule/Saturday")) {
		tempSchedule[5] = firebaseData.stringData();
	}
	if (Firebase.getString(firebaseData, "/Schedule/Sunday")) {
		tempSchedule[6] = firebaseData.stringData();
	}

	if (Mode) { // Mode is in auto
		String Day = getDay();
		String Schedule;
		if (Day.equals("Monday")) {
			Schedule = tempSchedule[0];
		}
		else if (Day.equals("Tuesday")) {
			Schedule = tempSchedule[1];
		}
		else if (Day.equals("Wednesday")) {
			Schedule = tempSchedule[2];
		}
		else if (Day.equals("Thursday")) {
			Schedule = tempSchedule[3];
		}
		else if (Day.equals("Friday")) {
			Schedule = tempSchedule[4];
		}
		else if (Day.equals("Saturday")) {
			Schedule = tempSchedule[5];
		}
		else {
			Schedule = tempSchedule[6];
		}
		tempPump = CheckSchedule(Schedule);
	}

	bool ScheduleChanged = false;
	for (int n = 0; n < 7; n++) {
		if (tempSchedule[n].equals(localFireData.Schedule[n])) {
		}
		else {
			ScheduleChanged = true;
		}
	}

	if (LocalChangeMade) {
		return true;
	}
	if (tempPump != localFireData.Pump_Running || tempHeater != localFireData.Heater_Running || tempMode != localFireData.Mode || ScheduleChanged) {
		FirebaseChangeMade = true;
		Pump_Running = tempPump;
		Heater_Running = tempHeater;
		Mode = tempMode;
		for (int n = 0; n < 7; n++) {
			localFireData.Schedule[n] = tempSchedule[n];
		}
		return true; // returns if a change was detected
	}
	return false;
}

// Pushes values to update firebase
void UpdateFirebase() {
	// Set values
	if (LocalChangeMade) {
		localFireData.Heater_Running = Heater_Running;
		localFireData.Pump_Running = Pump_Running;
		LocalChangeMade = false;
	}
	else if (FirebaseChangeMade) {
		localFireData.Heater_Running = Heater_Running;
		localFireData.Pump_Running = Pump_Running;
		FirebaseChangeMade = false;
	}
	localFireData.WaterTemp = WaterTemp;
	localFireData.AirTemp = AirTemp;
	localFireData.pH = pH;
	localFireData.WaterLevel = WaterLevel;

	Serial.println("========================================");
	// Push Heater
	if (Firebase.setBool(firebaseData, "/Devices/Heater", localFireData.Heater_Running)) {
		Serial.println(firebaseData.dataPath() + " = " + localFireData.Heater_Running);
	}
	else {
		Serial.println("Error with: Heater");
		Serial.println("Error: " + firebaseData.errorReason());
	}

	// Push Filter / Pump
	if (Firebase.setBool(firebaseData, "/Devices/Pump", localFireData.Pump_Running)) {
		Serial.println(firebaseData.dataPath() + " = " + localFireData.Pump_Running);
	}
	else {
		Serial.println("Error with: Pump / Filter");
		Serial.println("Error: " + firebaseData.errorReason());
	}

	// Push Water Temp
	if (Firebase.setFloat(firebaseData, "/Sensors/WaterTemp", localFireData.WaterTemp)) {
		Serial.println(firebaseData.dataPath() + " = " + localFireData.WaterTemp);
	}
	else {
		Serial.println("Error with: Water Temp");
		Serial.println("Error: " + firebaseData.errorReason());
	}

	// Push Air Temp
	if (Firebase.setFloat(firebaseData, "/Sensors/AirTemp", localFireData.AirTemp)) {
		Serial.println(firebaseData.dataPath() + " = " + localFireData.AirTemp);
	}
	else {
		Serial.println("Error with: Air Temp");
		Serial.println("Error: " + firebaseData.errorReason());
	}

	// Push pH
	if (Firebase.setFloat(firebaseData, "/Sensors/pH", localFireData.pH)) {
		Serial.println(firebaseData.dataPath() + " = " + localFireData.pH);
	}
	else {
		Serial.println("Error with: pH");
		Serial.println("Error: " + firebaseData.errorReason());
	}

	// Push Water Level
	if (Firebase.setBool(firebaseData, "/Sensors/WaterLevel", localFireData.WaterLevel)) {
		Serial.println(firebaseData.dataPath() + " = " + localFireData.WaterLevel);
	}
	else {
		Serial.println("Error with: Water Level");
		Serial.println("Error: " + firebaseData.errorReason());
	}
	Serial.println("=======================================");
}

void UpdateRelays() {
	// This function will set pins HIGH or LOW to turn relays on or off
	if (localFireData.Pump_Running == true) {
		Pump_Running = true;
		digitalWrite(PUMPPIN, HIGH);
	}
	else {
		Pump_Running = false;
		digitalWrite(PUMPPIN, LOW);
	}
	if (localFireData.Heater_Running == true) {
		Heater_Running = true;
		digitalWrite(HEATERPIN, HIGH);

	}
	else {
		Heater_Running = false;
		digitalWrite(HEATERPIN, LOW);
	}
}

const int LEN = 40;
int pHArray[LEN] = { 0 };
int pHArrayIDX = 0;
static float voltage;
void ReadSensors() {
	// Air Temp
	AirTemp = AirTempSensor.readTemperature();
	AirTemp = (AirTemp * 1.8) + 32; // Convert to F

	// Water Temp
	WaterTempSensor.requestTemperatures();
	delay(25);
	WaterTemp = WaterTempSensor.getTempFByIndex(0);

	// pH
	for (int i = 0; i < 50; i++) {
		pHArray[pHArrayIDX++] = analogRead(pHSensorPin);
		if (pHArrayIDX == LEN) {
			pHArrayIDX = 0;
		}
		voltage = averagearray(pHArray, LEN) * 5.0 / 1024;
		pH = 3.5 * voltage + pHOFFSET;
		delay(10);
	}

	// Water Level
	WaterLevel = digitalRead(WATERLVL);

}

double averagearray(int* arr, int number) {
	int i;
	int max, min;
	double avg;
	long amount = 0;
	if (number <= 0) {
		Serial.println("Error number for the array to avraging!/n");
		return 0;
	}
	if (number < 5) {   //less than 5, calculated directly statistics
		for (i = 0; i < number; i++) {
			amount += arr[i];
		}
		avg = float(amount) / number;
		return avg;
	}
	else {
		if (arr[0] < arr[1]) {
			min = arr[0]; max = arr[1];
		}
		else {
			min = arr[1]; max = arr[0];
		}
		for (i = 2; i < number; i++) {
			if (arr[i] < min) {
				amount += min;        //arr<min
				min = arr[i];
			}
			else {
				if (arr[i] > max) {
					amount += max;    //arr>max
					max = arr[i];
				}
				else {
					amount += arr[i]; //min<=arr<=max
				}
			}//if
		}//for
		avg = (double)amount / (number - 2);
	}//if
	return avg;
}

// Keypad stuff
void keypadEvent(KeypadEvent key) {
	switch (customKeypad.getState()) {
	case PRESSED:
		if (key == '*') {
			Serial.println("pressed...");

			keypadEditMode = !keypadEditMode;
		}
		break;
	}
}

// More keypad stuff
void keypadEdit() {
	int selectedRow = 1; // Row that the user is editing
	while (keypadEditMode) {
		String Pump_Running_String, Heater_Running_String;
		if (!Pump_Running) {
			Pump_Running_String = "OFF";
		}
		else {
			Pump_Running_String = "ON";
		}
		if (!Heater_Running) {
			Heater_Running_String = "OFF";
		}
		else {
			Heater_Running_String = "ON";
		}
		const int ROWMAX = 2;
		int row = 1; // Row to be written to currently
		display.clearDisplay();
		display.setTextSize(1);
		display.setTextColor(BLUE);
		display.setCursor(1, 1);
		display.println("Edit Mode:");
		display.println("Relays:");
		if (selectedRow == row) {
			display.print("--> ");
		}
		display.print("Pump: "); display.println(Pump_Running_String);
		row += 1;
		if (selectedRow == row) {
			display.print("--> ");
		}
		display.print("Heater: "); display.println(Heater_Running_String);
		display.display();

		char key = customKeypad.waitForKey();
		if (key == '#') {
			LocalChangeMade = true;
			switch (selectedRow) {
			case 1:
				Pump_Running = !Pump_Running;
				break;
			case 2:
				Heater_Running = !Heater_Running;
				break;
			}
		}
		if (key == 'A') {
			selectedRow = 1;
		}
		if (key == 'B') {
			selectedRow = 2;
		}
	}
	display.clearDisplay();
}

////////////////////// START THE MACHINE
void setup() {

	pinMode(LED_BUILTIN, OUTPUT);
	pinMode(WATERLVL, INPUT);

	customKeypad.addEventListener(keypadEvent);

	WaterTempSensor.begin();
	AirTempSensor.begin();

	//while (!Serial) {
	//	; // wait for serial connection
	//}

	Wire.begin();

	// -------------------------------------------------------------
	// Setup display

	display.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS);
	display.display();
	display.clearDisplay();
	display.clearDisplay();
	display.display();
	display.drawPixel(0, 0, BLUE);
	display.drawPixel(127, 0, BLUE);
	display.drawPixel(0, 63, BLUE);
	display.drawPixel(127, 63, BLUE);
	display.setTextSize(1);
	display.setTextColor(BLUE);
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

void loop() {

	char key = customKeypad.getKey();
	if (keypadEditMode) {
		keypadEdit();
	}

	// -------------------------------------------------------------
	// Write Data to Display

	String Pump_Running_String, Heater_Running_String, Water_Level_String;
	if (!Pump_Running) {
		Pump_Running_String = "OFF";
	}
	else {
		Pump_Running_String = "ON";
	}
	if (!Heater_Running) {
		Heater_Running_String = "OFF";
	}
	else {
		Heater_Running_String = "ON";
	}
	if (!WaterLevel) {
		Water_Level_String = "OFF";
	}
	else {
		Water_Level_String = "ON";
	}

	display.clearDisplay();
	display.setTextSize(1);
	display.setCursor(1, 1);
	display.println(getTimeString());
	display.println("Sensors:");
	display.print("Air Tem: "); display.println(AirTemp);
	display.print("Water Temp: "); display.println(WaterTemp);
	display.print("pH: "); display.print(pH); display.print(" Level: "); display.println(Water_Level_String);
	display.println("Relays:");
	display.print("Pump: "); display.println(Pump_Running_String);
	display.print("Heater: "); display.println(Heater_Running_String);
	display.display();

	// -------------------------------------------------------------

	// -------------------------------------------------------------
	// Read sensor and firebase data and store into memory

	ChangeMade = GetFirebaseValues();
	ReadSensors(); // Read sensors and store their values

	// Data read in by this point
	// -------------------------------------------------------------

	// -------------------------------------------------------------
	// Upload updated data to firebase

	if (Update_Firebase_Counter > 20 || ChangeMade) // If Update_Firebase_Counter gets to 20, push values to firebase (about 40 seconds)
	{
		UpdateFirebase(); // Push updated values to Firebase
		Update_Firebase_Counter = 0;
	}

	// -------------------------------------------------------------

	// -------------------------------------------------------------
	// Make appropriate relay modifications

	UpdateRelays(); // Turn Relays on or off based on the data struct's values

	// -------------------------------------------------------------

	delay(500);
	Update_Firebase_Counter++;

	// With no manual updates, Firebase will be automatically updated every 41 seconds approximately.
}
