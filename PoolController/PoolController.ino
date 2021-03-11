#include <SPI.h>
#include <WiFiNINA.h>
#include <OneWire.h>
#include <DallasTemperature.h>
#include <Keypad.h>
#include "Firebase_Arduino_WiFiNINA.h"
#include "Firebase_Arduino_WiFiNINA_HTTPCLient.h"

FirebaseData firebaseData;

struct Data // struct to contain a copy of the database values
{
  bool Heater_Running = false;  // Relay 1
  bool Pump_Running = false;    // Relay 2
  float AirTemp = 0.0;          // Air Temp Probe
  float WaterTemp = 0.0;        // Water Temp Probe
  float pH = 7.0;               // pH Temp Probe
  float WaterLevel = 0.0;       // Water Level Probe
};
Data data;

const byte ROWS = 4; 
const byte COLS = 4; 

char hexaKeys[ROWS][COLS] = {
  {'1', '2', '3', 'A'},
  {'4', '5', '6', 'B'},
  {'7', '8', '9', 'C'},
  {'*', '0', '#', 'D'}
};
byte rowPins[COLS] = {3, 4, 5, 6};
byte colPins[ROWS] = {7, 8, 9, 10};

bool change = false;

Keypad customKeypad = Keypad(makeKeymap(hexaKeys), rowPins, colPins, ROWS, COLS); 

// PIN DEFINITIONS
#define WATER_SENS_PROBE_DATA 2
#define KEYPAD_R1 3
#define KEYPAD_R2 4
#define KEYPAD_R3 5
#define KEYPAD_R4 6
#define KEYPAD_C1 7
#define KEYPAD_C2 8
#define KEYPAD_C3 9
#define KEYPAD_C4 10

// Defines
#define WIFI_SSID "Josiah's S20+"
#define WIFI_PASS "7205858843"
//#define WIFI_SSID "Luke's iPhone"
//#define WIFI_PASS "mynamejeff"
#define FIREBASE_HOST "es-pool-controller-default-rtdb.firebaseio.com"
#define FIREBASE_AUTH "YXsdNJ9OFISN2ZwgUhIQxny6KDtDgdMdm9Ho6HWL"

  OneWire oneWire(WATER_SENS_PROBE_DATA);
  DallasTemperature sensors(&oneWire);

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

// Pushes values to update firebase
void UpdateFirebase() {
  // Push Heater
  if (Firebase.setBool(firebaseData, "/Devices/Heater", data.Heater_Running)) {
      Serial.println(firebaseData.dataPath() + " = " + data.Heater_Running);
    }
  else {
    Serial.println("Error: " + firebaseData.errorReason());
  }
  
  // Push Filter / Pump
  if (Firebase.setBool(firebaseData, "/Devices/Pump", data.Pump_Running)) {
      Serial.println(firebaseData.dataPath() + " = " + data.Pump_Running);
    }
  else {
    Serial.println("Error: " + firebaseData.errorReason());
  }

  // Push Water Temp
  if (Firebase.setFloat(firebaseData, "/Sensors/WaterTemp", data.WaterTemp)) {
      Serial.println(firebaseData.dataPath() + " = " + data.WaterTemp);
    }
  else {
    Serial.println("Error: " + firebaseData.errorReason());
  }

  // Push Air Temp
  if (Firebase.setFloat(firebaseData, "/Sensors/AirTemp", data.AirTemp)) {
      Serial.println(firebaseData.dataPath() + " = " + data.AirTemp);
    }
  else {
    Serial.println("Error: " + firebaseData.errorReason());
  }

  // Push pH
  if (Firebase.setFloat(firebaseData, "/Sensors/pH", data.pH)) {
      Serial.println(firebaseData.dataPath() + " = " + data.pH);
    }
  else {
    Serial.println("Error: " + firebaseData.errorReason());
  }

  // Push Water Level
  if (Firebase.setFloat(firebaseData, "/Sensors/WaterLevel", data.WaterLevel)) {
      Serial.println(firebaseData.dataPath() + " = " + data.WaterLevel);
    }
  else {
    Serial.println("Error: " + firebaseData.errorReason());
  }
}

void UpdateRelays() {
  
}

void ReadSensors() {
  // Air Temp


  // Water Temp
  sensors.requestTemperatures();
  delay(25);
  data.WaterTemp = sensors.getTempFByIndex(0) + 2.1; // Offset comes from adjusting temp probe readings

  // pH


  // Water Level

  
}

void KeypadInterrupt() {
  Serial.println("INTERRUPT!");
  char customKey = customKeypad.getKey();

  if (customKey){
    Serial.println(customKey);
  }
}

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

void setup() {

  pinMode(LED_BUILTIN, OUTPUT);

  customKeypad.addEventListener(keypadEvent);
  
  sensors.begin();
  
  while(!Serial) {
    ; // wait for serial connection
    }

  // -------------------------------------------------------------
  // Start Wifi & Connect to Firebase

  SetupWifiConnection();

  while(wifiStatus != WL_CONNECTED) {
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
  if (change == true) getnum();
  
  // -------------------------------------------------------------
  // Read sensor and firebase data and store into memory

  ReadSensors(); // Read sensors and store their values
  
  // Data read in by this point
  // -------------------------------------------------------------
  
  // -------------------------------------------------------------
  // Make appropriate relay modifications

  UpdateRelays(); // Turn Relays on or off based on the data struct's values

  // -------------------------------------------------------------

  // -------------------------------------------------------------
  // Upload updated data to firebase

  UpdateFirebase(); // Push updated values to Firebase

  // -------------------------------------------------------------
  delay(500);
}
