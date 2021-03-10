#include <SPI.h>
#include <WiFiNINA.h>
#include "Firebase_Arduino_WiFiNINA.h"

FirebaseData firebaseData;

struct Data // struct to contain a copy of the database values
{
  bool Heater_Running = false;  // Relay 1
  bool Pump_Running = false;    // Relay 2
  float AirTemp = 0.0;          // Air Temp Probe
  float WaterTemp = 0.0;        // Water Temp Probe
  float pH = 7.0;               // pH Temp Probe
};
Data data;

// PIN DEFINITIONS


// Defines
char C_SSID[100] = "OCguest";
char C_PASS[100] = {NULL};

String S_SSID = C_SSID;
String S_PASS = C_PASS;
String FIREBASE_ADDR = "es-pool-controller-default-rtdb.firebaseio.com"; // firebase database url
String FIREBASE_SECRET = "YXsdNJ9OFISN2ZwgUhIQxny6KDtDgdMdm9Ho6HWL"; // database secret code

int wifiStatus;

// functions to turn on or off the builtin LED
  void LEDOn() {
    digitalWrite(LED_BUILTIN, HIGH);
  }
  
  void LEDOff() {
    digitalWrite(LED_BUILTIN, LOW);
  }

// Pushes values to update firebase
void UpdateFirebase() {
  // Push Heater
  String path = "/Devices";
  if (Firebase.setBool(firebaseData, path + "/Heater", data.Heater_Running)) {
      Serial.println(firebaseData.dataPath() + " = " + data.Heater_Running);
    }
  else {
    Serial.println("Error: " + firebaseData.errorReason());
  }
}


// Attempts a wifi connection based upon the contents of ssid and pass
void SetupWifiConnection() {  
  LEDOff();
  if (strcmp(C_PASS, NULL) == 0) {  // If password field is NULL, attempt login without pass
    wifiStatus = WiFi.begin(C_SSID);
  }
  else {
    wifiStatus = WiFi.begin(C_SSID, C_PASS); // Attempt login with pass
  }
}

void setup() {

  while(!Serial) {
    ; // wait for serial connection
    }
  
  pinMode(LED_BUILTIN, OUTPUT);
  
  // Setup WiFi connections, FireBase connection, and sensor profiles

  // -------------------------------------------------------------
  // Start Wifi & Connect to Firebase

  SetupWifiConnection();

  while(wifiStatus != WL_CONNECTED) {
    SetupWifiConnection();
    delay(10000); // Delay for 10 seconds
  }
  
  LEDOn();
  Serial.println(); 
  Serial.print("Connected to ");
  Serial.print(C_SSID);
  Serial.print(" as: "); 
  Serial.println(WiFi.localIP());
  

  Firebase.begin(FIREBASE_ADDR, FIREBASE_SECRET, S_SSID, S_PASS); // Connect to firebase
  Firebase.reconnectWiFi(true);

  // Wifi & Firebase should be connected
  // -------------------------------------------------------------

  // -------------------------------------------------------------
  // Setup Pins regarding any pull-up nature or pull-down nature they require
  
  
  
}

void loop() {
  // -------------------------------------------------------------
  // Read sensor and firebase data and store into memory

  UpdateFirebase();

  // Data read in
  // -------------------------------------------------------------
  
  // -------------------------------------------------------------
  // Make appropriate relay modifications

  

  // -------------------------------------------------------------

  // -------------------------------------------------------------
  // Upload updated data to firebase



  // -------------------------------------------------------------
  delay(2000); // Delay 2 seconds
}
