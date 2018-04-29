#include <ArduinoOTA.h>


const char* OTApass     = "wordclock-OTA";

void OTA_setup(){

    ArduinoOTA.setPassword(OTApass);

    ArduinoOTA.onStart([]() {
      Serial.println("OTA Start");
    });
    ArduinoOTA.onProgress([](unsigned int progress, unsigned int total) {
      Serial.printf("OTA Progress: %u%%\r", (progress / (total / 100)));
    });
    ArduinoOTA.onEnd([]() {
      Serial.println("OTA End");
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

