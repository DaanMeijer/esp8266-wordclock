
#include <vector>

#include <ESP8266WiFi.h>
#include <ESP8266mDNS.h>
#include <WiFiManager.h>

#include <ArduinoOTA.h>

#include <TimeLib.h>

#include <EEPROM.h>


#define DEBUG_GENERAL 0
#define DEBUG_SCHEDULER 0

#define SCREEN_DATA_PIN  D1 // D2 Pin on Wemos mini
#define NUM_LEDS 100

#include "AnimatedPixels.h"

struct {
  time_t utc = 0;
  byte brightness = 255;
  
} persistent;

AnimatedPixels * screen;

void savePersistent(){
  Serial.printf("Saving persistent: brightness (%d), utc (%d)\n", persistent.brightness, persistent.utc);
  EEPROM.put(0, persistent);
}

void loadPersistent(){
  EEPROM.get(0, persistent);
  Serial.printf("Loading persistent: brightness (%d), utc (%d)\n", persistent.brightness, persistent.utc);
  
}

void screenTick(){
  #if DEBUG_GENERAL
  Serial.println("screenTick()");
  #endif
    
  int prefFrame = millis();
  
  screen->tick();
  
  int millisPassed = millis() - prefFrame;
  Serial.printf("Millis for a frame: %d\r\n", millisPassed);
}

std::vector<std::vector<int>> hourVectors = {
  {43,58,63,78,83,98}, //twaalf
  {24,23,22}, //een
  {35,46,55,66}, //twee
  { 5,16,25,36}, //drie
  { 3,18,23,38}, //vier
  {39,42,59,62}, //vijf
  {75,86,95}, //zes
  {17,24,37,44,57}, //zeven
  { 6,15,26,35}, //acht
  {57,64,77,84,97}, //negen
  {65,76,85,96}, //tien
  {36,45,56}, //elf
  {43,58,63,78,83,98}, //twaalf
};

std::vector<std::vector<int>> minuteVectors = {
  
  {79,82,99}, //uur
  {12,29,32,49,  68,73,88,93}, //vijf over
  {52,69,72,89,  68,73,88,93}, //tien over
  
  {8,13,28,33,48,  68,73,88,93}, //kwart over
  {52,69,72,89,  14,27,34,47,  54,67,74,87}, //tien voor half
  {12,29,32,49,  14,27,34,47,  54,67,74,87}, //vijf voor half
  
  {54,67,74,87}, //half
  {12,29,32,49,  68,73,88,93,  54,67,74,87}, //vijf over half
  {52,69,72,89,  68,73,88,93,  54,67,74,87}, //tien over half
  
  {8,13,28,33,48,  14,27,34,47}, //kwart voor
  {52,69,72,89,    14,27,34,47}, //tien voor
  {12,29,32,49,    14,27,34,47}, //vijf voor
  
};

std::vector<std::vector<int>> looseMinuteVectors = {
  {},
  {40},
  {40,41},
  {40,41,60},
  {40,41,60,61},
};




void renderSpecificTime(int relevantHour, int minutes, int seconds){
  if(seconds == 0){
    screen->showSettings();
  }
//  Serial.println("renderSpecificTime()");

//  Serial.print("Current time: ");
//  Serial.print(relevantHour);
//  Serial.print(":");
//  Serial.println(minutes);

  screen->clear();

  
//  Serial.println("cleared the leds");

  /**/
  
  std::vector<int> staticLeds = {
    10,11,30,  50,51,  71,90   //het is nu
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

  int looseMinutes = minutes%5;
  std::vector<int> looseMinuteLeds = looseMinuteVectors[looseMinutes];

  for(int i=0; i<looseMinuteLeds.size(); i++){
    int pos = looseMinuteLeds[i] - 1;
    screen->on(pos);
  }
  
}


void renderTime(){
//  Serial.println("renderTime()");
  
  int _hour = hour();
  int minutes = minute();

  int relevantHour = minutes < 20 ? _hour : _hour+1;

  while(relevantHour > 12){
    relevantHour -= 12;
  }

  renderSpecificTime(relevantHour, minutes, second());
  
//  Serial.println("renderTime done");
}

void callback(byte* payload, unsigned int length) {

  Serial.print("Payload: ");
  for(int a=0; a<length; a++){
    Serial.print((char)payload[a]);
  }
  Serial.println();

  switch(payload[0]){
      case 'b':
      persistent.brightness = payload[1];
      savePersistent();
      screen->setBrightness(persistent.brightness * 1.0f);
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

    EEPROM.begin(512);

    delay(10);

    loadPersistent();

    Serial.printf("setting time to %d\n", persistent.utc);
    setTime(persistent.utc);

    screen = new AnimatedPixels(NUM_LEDS);
    screen->setBrightness(persistent.brightness * 1.0f);    

    addFunction(screenTick, 20);

    addFunction(renderTime, 1000);

    addFunction(Time_loop, 1000);

    startWiFi();

    MQTT_init();
    MQTT_subscribe("cmnd/WordClock/Dimmer", [](byte* payload, unsigned int length){
      auto input = new String((char *)payload);
      Serial.println(input->c_str());
      auto brightness = input->toInt();
      delete input;
      screen->setBrightness(brightness * 1.0f);
    });

    MQTT_subscribe("cmnd/WordClock/Mode", [](byte* payload, unsigned int length){
      char buff[length + 1];
      strncpy(buff, (char*)payload, length);
      buff[length] = 0;
      
      auto input = new String(buff);
      input->toLowerCase();
      
      if(input->equals("rainbow")){
        screen->rainbow();
      }else if(input->equals("random")){
        screen->randomize();
      }else if(input->equals("jitter")){
        screen->jitterize();
      }else if(input->equals("synchronous")){
        screen->synchronize();
      }else if(input->equals("mono")){
        screen->mono();
      }else if(input->equals("gradient")){
        screen->gradientize();
      }
      
      delete input;
    });

    MQTT_subscribe("cmnd/WordClock/Color", [](byte* payload, unsigned int length){
      char buff[length + 1];
      strncpy(buff, (char*)payload, length);
      buff[length] = 0;
 

      long rgb = strtol(buff, NULL, 16);
      byte red = rgb >> 16 ;
      byte green = (rgb & 0x00ff00) >> 8;
      byte blue = (rgb & 0x0000ff);
      
      screen->setRGB(red, green, blue);
      
    });

    
    MQTT_subscribe("cmnd/WordClock/Render", [](byte* payload, unsigned int length){
      renderTime();
    });
    Serial.println("MQTT_init done");
    
    OTA_setup();

    Time_setup();
    
    
    ArduinoOTA.onStart([]() {
      screen->setBrightness(64.0f);
      screen->clear();
      screen->on(0, true);
      screen->render();
    });

    ArduinoOTA.onProgress([](unsigned int progress, unsigned int total) {
      
      float complete = (float)progress / (float)total;
      uint8_t pos = complete * NUM_LEDS * 1.0f;
      if(pos > NUM_LEDS - 1){
        pos = NUM_LEDS - 1;
      }
      
      screen->on(pos, true);
      screen->tick();
      
      Serial.printf("OTA Progress: %f%%\r", complete * 100.0f);
      
      ESP.wdtFeed();

    });
    
    ESP.wdtEnable(1000);
    
    MQTT_publish("wordclock", "!Wordclock online!");
    Serial.println("MQTT message sent");
    
}


// the loop function runs over and over again forever
void loop() {

  #if DEBUG_GENERAL
  Serial.println("loop()");
  #endif
  
  ESP.wdtFeed();

  
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
