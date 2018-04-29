#include <NTPClient.h>


#include <Time.h>
#include <Timezone.h>



WiFiUDP ntpUdpClient;

NTPClient timeClient(ntpUdpClient, "europe.pool.ntp.org", 3600, 60000);

TimeChangeRule CEST = {"CEST", Last, Sun, Mar, 2, 120};     //Central European Time (Frankfurt, Paris)
TimeChangeRule CET = {"CET ", Last, Sun, Oct, 3, 60};       //Central European Time (Frankfurt, Paris)
Timezone CE(CEST, CET);

time_t syncProvider(){
  Serial.print("syncProvider: ");
  Time_loop();
  
  time_t utc = timeClient.getEpochTime();
  time_t local = CE.toLocal(utc);
  Serial.printf("utc: %d, local: %d\n", utc, local);
  
  return local;
}

void Time_setup(){
  
  timeClient.begin();
  
  setSyncProvider(syncProvider);
  //Set Sync Intervals
  setSyncInterval(1);

    
}

void Time_loop(){
  
  Serial.println(timeClient.getFormattedTime());
  timeClient.update();

}

