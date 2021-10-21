#include <NTPClient.h>


#include <Time.h>
#include <Timezone.h>



WiFiUDP ntpUdpClient;

NTPClient timeClient(ntpUdpClient, "europe.pool.ntp.org");

TimeChangeRule CEST = {"CEST", Last, Sun, Mar, 2, 120};     //Central European Time (Frankfurt, Paris)
TimeChangeRule CET = {"CET ", Last, Sun, Oct, 3, 60};      //Central European Time (Frankfurt, Paris)
Timezone CE(CEST, CET);

void print_time(time_t tt) {
    char buf[80];
    struct tm* st = localtime(&tt);
    strftime(buf, 80, "%c", st);
    Serial.printf("%s", buf);
}

time_t syncProvider(){
  
  Serial.print("syncProvider: ");
  Time_loop();
  
  time_t utc = timeClient.getEpochTime();
  time_t local = CE.toLocal(utc);
  Serial.print("utc: ");
  print_time(utc);
  Serial.print(", local: ");
  print_time(local);
  Serial.println();
  
  persistent.utc = utc;
  savePersistent();

  return local;
}

void Time_setup(){
  
  timeClient.begin();
  timeClient.forceUpdate();

  MQTT_publish("wordclock", String(timeClient.getEpochTime()));
  setTime(timeClient.getEpochTime());
  
  setSyncProvider(syncProvider);
  //Set Sync Intervals
  setSyncInterval(300);

  Time_loop();
}

void Time_loop(){
  
//  Serial.println(timeClient.getFormattedTime());
  timeClient.update();

}
