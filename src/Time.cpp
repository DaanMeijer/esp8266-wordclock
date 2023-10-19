
#include <TimeLib.h>

#include <NTPClient.h>
#include <Timezone.h>

#include <ESP8266WiFi.h>
#include <WiFiUdp.h>

WiFiUDP ntpUdpClient;

NTPClient timeClient(ntpUdpClient, "europe.pool.ntp.org", 3600, 60000);

TimeChangeRule CEST = {"CEST", Last, Sun, Mar, 2, 60};     //Central European Time (Frankfurt, Paris)
TimeChangeRule CET = {"CET ", Last, Sun, Oct, 3, 0};      //Central European Time (Frankfurt, Paris)
Timezone CE(CEST, CET);


void Time_loop(){
  
//  Serial.println(timeClient.getFormattedTime());
  timeClient.update();

}


time_t syncProvider(){



  Serial.println("Getting a new time");
  Time_loop();
  
  time_t utc = timeClient.getEpochTime();
  setTime(utc);
  time_t local = CE.toLocal(utc);

  Serial.printf("local: %d/%d/%d %d:%d:%d\n", day(local), month(local), year(local), hour(local), minute(local), second(local));

  return local;
}

void Time_setup(){
  
  timeClient.begin();
  timeClient.forceUpdate();

  //MQTT_publish("wordclock", String(timeClient.getEpochTime()));
  setTime(timeClient.getEpochTime());
  
  setSyncProvider(syncProvider);
  //Set Sync Intervals
  setSyncInterval(30);

  Time_loop();
}

