#include <ESP8266WiFi.h>

#include <ESPAsync_WiFiManager.h>
#include <Arduino.h>

#include "WifiSettings.h"



void Wifi_setup_softap(){

  Serial.print("Setting soft-AP ... ");
  boolean result = WiFi.softAP("ESPsoftAP_01", "pass-to-soft-AP");
  if(result == true)
  {
    Serial.println("Ready");
  }
  else
  {
    Serial.println("Failed!");
  }

}

void Wifi_setup(){

    WiFi.mode(WIFI_STA);
    //TODO:
    WiFi.begin(WIFI_ESSID, WIFI_PASSWORD);

     while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        Serial.print(".");
    }

    Serial.println("");
    Serial.println("WiFi connected");
    Serial.println("IP address: ");
    Serial.println(WiFi.localIP());

}

