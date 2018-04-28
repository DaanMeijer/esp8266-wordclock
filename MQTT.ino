#include <ESP8266WiFi.h>
#include <PubSubClient.h>

WiFiClient espClient;
PubSubClient client(espClient);

char * topic;

void reconnect() {
  // Loop until we're reconnected
  while (!client.connected()) {
//    Serial.print("Attempting MQTT connection...");
    // Create a random client ID
    String clientId = "ESP8266Client-";
    clientId += String(random(0xffff), HEX);
    // Attempt to connect
    if (client.connect(clientId.c_str())) {
//      Serial.println("connected");

      // ... and resubscribe
      if(topic != 0){
        client.subscribe(topic);
      }
    } else {
//      Serial.print("failed, rc=");
//      Serial.print(client.state());
//      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}

const char* mqtt_server = "192.168.1.1";


void MQTT_publish(const char * msg){
//  Serial.print("Publish message: ");
//  Serial.println(msg);
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

//  Serial.println("MQTT_init");
  
  client.setServer(mqtt_server, 1883);
  client.setCallback(MQTT_callback);
  
//  Serial.println("MQTT_init after callback");
  char buff[256];
  sprintf(buff, "%08x", _pCallback);
//  Serial.println(buff);

  pCallback = _pCallback;

  
}

void callback(char* topic, byte* payload, unsigned int length) {
//  Serial.print("Message arrived [");
//  Serial.print(topic);
//  Serial.print("] ");
//  for (int i = 0; i < length; i++) {
//    Serial.print((char)payload[i]);
//  }
//  Serial.println();

  // Switch on the LED if an 1 was received as first character
  if ((char)payload[0] == '1') {
    digitalWrite(BUILTIN_LED, LOW); // Turn the LED on (Note that LOW is the voltage level
    // but actually the LED is on; this is because
    // it is acive low on the ESP-01)
  } else {
    digitalWrite(BUILTIN_LED, HIGH); // Turn the LED off by making the voltage HIGH
  }
}

void MQTT_loop(){
  if (!client.connected()) {
    reconnect();
  }
  client.loop();
}

