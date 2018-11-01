#include <ESP8266WiFi.h>
#include <PubSubClient.h>

WiFiClient espClient;
PubSubClient client(espClient);

char * topic;

void connect() {
  
  Serial.print("Attempting MQTT connection...");
  // Create a random client ID
  String clientId = "ESP8266Client-";
  clientId += String(random(0xffff), HEX);
  // Attempt to connect
  if (client.connect(clientId.c_str())) {
    Serial.println("connected");

    // ... and resubscribe
    if(topic != NULL){
      client.subscribe(topic);
    }
  } else {
    Serial.print("failed, rc=");
    Serial.println(client.state());

  }
}

const char* mqtt_server = "192.168.1.1";


void MQTT_publish(String str){
  char * buff = new char[str.length() + 1];
  str.toCharArray(buff, str.length() + 1);
  MQTT_publish(buff);
  delete buff;
}

void MQTT_publish(const char * msg){
  client.publish(topic, msg);
}

void (*pCallback)(byte *, unsigned int) = NULL;





void MQTT_callback(char* _topic, byte * payload, unsigned int length){
//  Serial.println("Have MQTT message");
  if(String(topic).equals(_topic)){
//    Serial.println("It's our topic");
    if(pCallback != NULL){
      pCallback(payload, length);
    } 
  }
}

void MQTT_init(const char * _topic, void (*_pCallback)(byte *, unsigned int)){
  topic = new char[strlen(_topic) + 1];
  memcpy(topic, _topic, strlen(_topic)+1);

  Serial.println("MQTT_init");
  Serial.printf("Topic: [%s]\n", topic);
  
  client.setServer(mqtt_server, 1883);
  client.setCallback(MQTT_callback);
  
//  Serial.println("MQTT_init after callback");
  char buff[256];
  sprintf(buff, "%08x", _pCallback);
//  Serial.println(buff);

  pCallback = _pCallback;

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

