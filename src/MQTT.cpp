#include "MQTT.hpp"

#include <PubSubClient.h>


#include <ESP8266WiFi.h>
#include <WiFiUdp.h>


WiFiClient espClient;
PubSubClient client(espClient);


std::vector<SubscribedChannel> * channels = new std::vector<SubscribedChannel>();

char mqtt_server[40] = "manus";
char mqtt_port[6] = "1883";

void connect() {
  
  Serial.print("Attempting MQTT connection...");
  // Create a random client ID
  String clientId = "Konijntje-";
  clientId += String(ESP.getChipId());
  // Attempt to connect
  if (client.connect(clientId.c_str())) {
    Serial.println("connected");

    // ... and resubscribe
//    if(topic != NULL){
//      client.subscribe(topic);
//    }
  } else {
    Serial.print("failed, rc=");
    Serial.println(client.state());

  }
}

void MQTT_publish(const char * topic, String str){
  char * buff = new char[str.length() + 1];
  str.toCharArray(buff, str.length() + 1);
  MQTT_publish(topic, buff);
  delete buff;
}

void MQTT_publish(const char * topic, const char * msg){
  client.publish(topic, msg);
}

void (*pCallback)(uint8_t *, unsigned int) = NULL;


void MQTT_subscribe(char * topic, void (*callback)(uint8_t *, unsigned int)){
  Serial.println("MQTT_subscribe");
  
  Serial.printf("Topic: [%s]\n", topic);
  
  SubscribedChannel * channel = new SubscribedChannel();
  channel->topic = new String(topic);
  channel->callback = callback;
  client.subscribe(topic);
  channels->push_back(*channel);
}



void MQTT_callback(char* topic, uint8_t * payload, unsigned int length){

  Serial.println("MQTT_callback");
  
  for (auto channel = channels->begin(); channel != channels->end(); ++channel){  
    if(channel->topic->equals(topic)){
      channel->callback(payload, length);
    }
  }

}


void MQTT_setup(){

  Serial.println("MQTT_init");
  
  client.setServer(mqtt_server, String(mqtt_port).toInt());
  client.setCallback(MQTT_callback);
  
//  Serial.println("MQTT_init after callback");
//  char buff[256];
//  sprintf(buff, "%08x", _pCallback);
//  Serial.println(buff);

  connect();
}

void MQTT_loop(){

  #if DEBUG_GENERAL
  Serial.println("MQTT_loop()");
  #endif
  int reconnectCounter = 0;
  while(!client.connected() && reconnectCounter < 3) {
    Serial.println("MQTT: connecting...");
    reconnectCounter++;
    connect();

    if(client.connected()){
      Serial.println("MQTT: succes!");
      break;
    }else{
      Serial.println("MQTT: connection failed, try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
  client.loop();
}
