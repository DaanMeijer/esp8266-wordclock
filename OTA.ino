#include <ArduinoOTA.h>


const char* OTApass     = "wordclock-OTA";

void doOta(){

    ArduinoOTA.setPassword(OTApass);

    ArduinoOTA.onStart([]() {
      Serial.println("OTA Start");
      for(int i=0;i<NUM_LEDS;i++) {
          //targetlevels[i] = 0;
          //currentlevels[i] = 0;
          leds[i] = CRGB::Black;
      }
      leds[0] = CRGB::Red;
  
      FastLED.show();
    });
    ArduinoOTA.onEnd([]() {
      Serial.println("OTA End");
    });
    ArduinoOTA.onProgress([](unsigned int progress, unsigned int total) {
      
      float complete = (float)progress / (float)total;
      uint8_t pos = complete * 94.0f;
      if(pos > 93){
        pos = 93;
      }
      
      leds[pos] = CRGB(random(0, 0xffffff));
      FastLED.show();
      
      Serial.printf("OTA Progress: %u%%\r", (progress / (total / 100)));
    });
    ArduinoOTA.onError([](ota_error_t error) {
      Serial.printf("OTA Error[%u]: ", error);
      if (error == OTA_AUTH_ERROR) Serial.println("Auth Failed");
      else if (error == OTA_BEGIN_ERROR) Serial.println("Begin Failed");
      else if (error == OTA_CONNECT_ERROR) Serial.println("Connect Failed");
      else if (error == OTA_RECEIVE_ERROR) Serial.println("Receive Failed");
      else if (error == OTA_END_ERROR) Serial.println("End Failed");
      ESP.restart();
    });
    
    ArduinoOTA.begin();

}

void OTA_loop(){
  ArduinoOTA.handle();
}

