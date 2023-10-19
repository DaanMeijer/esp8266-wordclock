
#include <vector>

#include <ESP8266WiFi.h>
#include <ESP8266mDNS.h>
#include <WiFiManager.h>

#include <ArduinoOTA.h>

#include <TimeLib.h>

#include <EEPROM.h>


#define DEBUG_GENERAL 0
#define DEBUG_SCREEN 1
#define DEBUG_SCHEDULER 0

#define SCREEN_DATA_PIN  D1 // D2 Pin on Wemos mini
#define NUM_LEDS 100

#include "AnimatedPixels.h"


AnimatedPixels * screen;


void screenTick(){
  #if DEBUG_SCREEN
  Serial.println("screenTick()");
  #endif
    
  #if DEBUG_SCREEN
  int prefFrame = millis();
  #endif
  
  screen->tick();
  
  #if DEBUG_SCREEN
  int millisPassed = millis() - prefFrame;
  Serial.printf("Millis for a frame: %d\r\n", millisPassed);
  #endif
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


    delay(10);

    loadPersistent();

    persistent.brightness = 128;

    //Serial.printf("setting time to %d\n", persistent.utc);
    //setTime(persistent.utc);
    Serial.printf("Setting brightness to %d\n", persistent.brightness);

    screen = new AnimatedPixels(NUM_LEDS, persistent.brightness * 1.0f);

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
      
      persistent.brightness = brightness;
      savePersistent();
      
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
      byte red =   (rgb & 0xff0000) >> 16 ;
      byte green = (rgb & 0x00ff00) >> 8;
      byte blue =  (rgb & 0x0000ff) >> 0;
      
      screen->setRGB(red, green, blue);
      
    });

    
    MQTT_subscribe("cmnd/WordClock/Render", [](byte* payload, unsigned int length){
      renderTime();
    });
    Serial.println("MQTT_init done");
    
    OTA_setup();

    Time_setup();
    
    MQTT_publish("wordclock", "!Wordclock online!");
    Serial.println("MQTT message sent");

    screen->clear(true);
    
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
