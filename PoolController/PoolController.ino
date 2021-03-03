#include <SPI.h>
#include <WiFiNINA.h>

// PIN DEFINITIONS


//SSID of network
char ssid[100] = "OCguest";
char pass[100] = {NULL};
int wifiStatus;

// Attempts a wifi connection based upon the contents of ssid and pass
void SetupWifiConnection() {  
  if (strcmp(pass, NULL) == 0) {  // If password field is NULL, attempt login without pass
    wifiStatus = WiFi.begin(ssid);
  }
  else {
    wifiStatus = WiFi.begin(ssid, pass); // Attempt login with pass
  }
}

void setup() {
  // Setup WiFi connections, FireBase connection, and sensor profiles

  // -------------------------------------------------------------
  // Start Wifi

  SetupWifiConnection();
  
  while(wifiStatus != WL_CONNECTED) {
    SetupWifiConnection();
    delay(10000); // Delay for 10 seconds
  }

  // Wifi should be connected
  // -------------------------------------------------------------


  // -------------------------------------------------------------
  // Setup Pins
  
  
  
}

void loop() {
  // Code for checking sensors and reacting accordingly






}
