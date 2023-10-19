
#pragma once

#include "settings.h"

#include "Arduino.h"

using namespace std;



struct SubscribedChannel {
  String * topic;
  void (*callback)(uint8_t *, unsigned int);
};




void MQTT_publish(const char * topic, String str);
void MQTT_publish(const char * topic, const char * msg);
void MQTT_loop();
void MQTT_setup();
