
#include <Syslog.h>

#include <vector>

#include <ESP8266WiFi.h>
#include <ESP8266mDNS.h>
#include <WiFiManager.h>

#include <ArduinoOTA.h>

#include <TimeLib.h>


#define DEBUG_SCHEDULER 0

#define SCREEN_DATA_PIN  D1 // D2 Pin on Wemos mini
#define NUM_LEDS 94

#include "AnimatedPixels.h"



WiFiUDP udpClient;
Syslog syslog(udpClient, "192.168.1.1", 514, "wordclock", "main", LOG_KERN);

AnimatedPixels * screen = new AnimatedPixels(NUM_LEDS);

void screenTick(){
  screen->tick();
}

std::vector<std::vector<int>> hourVectors = {
  {}, //0, skip het
  {66,75,86}, //een
  {74,75,76,77}, //twee
  {41,42,43,44}, //drie
  {60,59,58,57}, //vier
  {82,83,84,85}, //vijf
  {71,72,73}, //zes
  {61,62,63,64,65}, //zeven
  {77,78,79,80}, //acht
  {65,66,67,68,69}, //negen
  {47,48,49,50}, //tien
  {44,45,46}, //elf
  {51,52,53,54,55,56}, //twaalf
};

std::vector<std::vector<int>> minuteVectors = {
  
  {88,89,90}, //uur
  {19,18,17,16,  27,28,29,30}, //vijf over
  {15,14,13,12,  27,28,29,30}, //tien over
  
  {21,22,23,24,25,  27,28,29,30}, //kwart over
  {15,14,13,12,  40,39,38,37,  35,34,33,32}, //tien voor half
  {19,18,17,16,  40,39,38,37,  35,34,33,32}, //vijf voor half
  
  {35,34,33,32}, //half
  {19,18,17,16,  27,28,29,30,  35,34,33,32}, //vijf over half
  {15,14,13,12,  27,28,29,30,  35,34,33,32}, //tien over half
  
  {21,22,23,24,25,  40,39,38,37}, //kwart voor
  {15,14,13,12,  40,39,38,37}, //tien voor
  {19,18,17,16,  40,39,38,37}, //vijf voor
  
};




void renderSpecificTime(int relevantHour, int minutes){
  

//  Serial.print("Current time: ");
//  Serial.print(relevantHour);
//  Serial.print(":");
//  Serial.println(minutes);

  screen->clear();

  
//  Serial.println("cleared the leds");

  /**/
  
  std::vector<int> staticLeds = {
    1,2,3,  5,6,  8,9
  };
  
  /*/
  
  std::vector<int> staticLeds = {
    1,2,3, 4, 5,6, 7, 8,9, 10,
    11,12,13, 14, 15,16, 17, 18,19, 20,
    21,22,23, 24, 25,26, 27, 28,29, 30,
    31,32,33, 34, 35,36, 37, 38,39, 40,
    41,42,43, 44, 45,46, 47, 48,49, 50,
    51,52,53, 54, 55,56, 57, 58,59, 60,
    61,62,63, 64, 65,66, 67, 68,69, 70,
    71,72,73, 74, 75,76, 77, 78,79, 80,
    81,82,83, 84, 85,86, 87, 88,89, 90,
    
    91,92,93, 94
  };

  /**/
  
  for(int i=0; i<staticLeds.size(); i++){
    int pos = staticLeds[i] - 1;
    screen->on(pos);
  }


  std::vector<int> hourLeds = hourVectors[relevantHour];
  
  for(int i=0; i<hourLeds.size(); i++){
    int pos = hourLeds[i] - 1;
    screen->on(pos);
  }
//  Serial.println("set the hour leds");

  int minuteIndex = minutes/5;
  std::vector<int> minuteLeds = minuteVectors[minuteIndex];

  for(int i=0; i<minuteLeds.size(); i++){
    int pos = minuteLeds[i] - 1;
    screen->on(pos);
  }
//  Serial.println("set the minute leds");

  for(int i=1; i<=minutes%5; i++){
    int pos = 94 - i;
//    Serial.print("setting dot ");
//    Serial.println(pos);
    screen->on(pos);
  }

//  Serial.print("seconds: ");
//  Serial.println(second());
  
}


void renderTime(){
  Serial.println("renderTime");
  

  
  syslog.log(LOG_DEBUG, "renderTime");
  int _hour = hour();
  int minutes = minute();

  int relevantHour = minutes < 20 ? _hour : _hour+1;

  while(relevantHour > 12){
    relevantHour -= 12;
  }

  renderSpecificTime(relevantHour, minutes);
  
//  Serial.println("renderTime done");
}

void callback(byte* payload, unsigned int length) {

//  Serial.print("Payload: ");
//  for(int a=0; a<length; a++){
//    Serial.print((char)payload[a]);
//  }
//  Serial.println();

  switch(payload[0]){
      case 'b':
      screen->setBrightness(payload[1] * 1.0f);
//      brightness = 255.0f;
      break;

      case 's':
      screen->setHueCycleInSeconds(payload[1] * 1.0f);
      break;

      case 'r':
      screen->rainbow();
      break;

      case 'S':
      screen->synchronize();
      break;
      
      case 'R':
      screen->randomize();
      break;

      case 'J':
      screen->jitterize();
      break;

      case 'G':
      screen->gradientize();
      break;
  }

}

void setup() {


    randomSeed(analogRead(0));

    Serial.begin(115200);
    for(int i=0; i<1; i++){
      Serial.println("Hello world.");
      delay(250);
    }
    
//    addFunction(animatedPixelTick, 1);
//    addFunction(renderAnimatedPixels, 10);
    

    addFunction(screenTick, 5);

    
    addFunction(renderTime, 1000);

    addFunction(Time_loop, 1000);

    startWiFi();
    Serial.println("OTA doing");
    
    OTA_setup();
    
    Serial.println("OTA done");

    Time_setup();
    
    MQTT_init("wordclock", callback);
    Serial.println("MQTT_init done");
    
    MQTT_publish("Hello Wordclock!");
    Serial.println("MQTT message sent");


    
    ArduinoOTA.onStart([]() {
      screen->clear();
      screen->on(0);
      screen->render();
    });

    ArduinoOTA.onProgress([](unsigned int progress, unsigned int total) {
      
      float complete = (float)progress / (float)total;
      uint8_t pos = complete * 94.0f;
      if(pos > 93){
        pos = 93;
      }
      
      screen->on(pos);
      screen->tick();
      
      Serial.printf("OTA Progress: %u%%\r", complete * 100.0f);
    });
}

// the loop function runs over and over again forever
void loop() {

  OTA_loop();
  MQTT_loop();
  
  Scheduler_loop();

}

void startWiFi(){
  
    WiFiManager wifiManager;

    String ssid = "WordClock-" + String(ESP.getChipId());
    wifiManager.autoConnect(ssid.c_str());
  
    Serial.println("Connected!");
    Serial.print("IP address: ");
    Serial.println(WiFi.localIP());
}

