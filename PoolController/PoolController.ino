#include <SPI.h>
#include <WiFiNINA.h>
#include "Firebase_Arduino_WiFiNINA.h"

FirebaseData firebaseData;

struct Data
{
  int Heater_Running = 0; // Relay 1
  int Pump_Running = 0;   // Relay 2
  float AirTemp = 0;      // Air Temp Probe
  float WaterTemp = 0;    // Water Temp Probe
  float pH = 7;           // pH Temp Probe
};
Data data;

// PIN DEFINITIONS


// Defines
String S_SSID = "OCguest";
String S_PASS = "";
String FIREBASE_ADDR = "https://es-pool-controller-default-rtdb.firebaseio.com/"; // firebase database url
String FIREBASE_SECRET = "YXsdNJ9OFISN2ZwgUhIQxny6KDtDgdMdm9Ho6HWL"; // database secret code

char C_SSID[100] = "OCguest";
char C_PASS[100] = {NULL};

int wifiStatus;

// Attempts a wifi connection based upon the contents of ssid and pass
void SetupWifiConnection() {  
  if (strcmp(C_PASS, NULL) == 0) {  // If password field is NULL, attempt login without pass
    wifiStatus = WiFi.begin(C_SSID);
  }
  else {
    wifiStatus = WiFi.begin(C_SSID, C_PASS); // Attempt login with pass
  }
}

void setup() {
  // Setup WiFi connections, FireBase connection, and sensor profiles

  // -------------------------------------------------------------
  // Start Wifi & Connect to Firebase

  SetupWifiConnection();
  
  while(wifiStatus != WL_CONNECTED) {
    SetupWifiConnection();
    delay(10000); // Delay for 10 seconds
  }
  Serial.println(); 
  Serial.print("Connected: "); 
  Serial.println(WiFi.localIP());

  Firebase.begin(FIREBASE_ADDR, FIREBASE_SECRET, S_SSID, S_PASS); // Connect to firebase

  // Wifi & Firebase should be connected
  // -------------------------------------------------------------

  // -------------------------------------------------------------
  // Setup Pins
  
  
  
}

void loop() {
  // -------------------------------------------------------------
  // Read sensor and firebase data and store into memory

  

  // Data read in
  // -------------------------------------------------------------
  
  // -------------------------------------------------------------
  // Make appropriate relay modifications

  

  // -------------------------------------------------------------

  // -------------------------------------------------------------
  // Upload updated data to firebase



  // -------------------------------------------------------------

}
