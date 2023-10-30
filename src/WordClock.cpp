
#include "Time.hpp"

#include <Arduino.h>


#define _TASK_SLEEP_ON_IDLE_RUN  // Enable 1 ms SLEEP_IDLE powerdowns between runs if no callback methods were invoked during the pass
#define _TASK_STATUS_REQUEST     // Compile with support for StatusRequest functionality - triggering tasks on status change events in addition to time only

#include <TaskScheduler.h>
#include <TimeLib.h>

#include "MQTT.hpp"
#include "Wifi.h"
#include "HTTP.hpp"

#include "Screen.hpp"


int prevMinute = -1;


#define NUM_LEDS 100



std::vector<std::vector<int>> hourVectors = {
  {43,58,63,78,83,98}, //twaalf
  {24,23,22}, //een
  {35,46,55,66}, //twee
  { 5,16,25,36}, //drie
  { 3,18,23,38}, //vier
  {39,42,59,62}, //vijf
  {75,86,95}, //zes
  {17,24,37,44,57}, //zeven
  { 6,15,26,35}, //acht
  {57,64,77,84,97}, //negen
  {65,76,85,96}, //tien
  {36,45,56}, //elf
  {43,58,63,78,83,98}, //twaalf
};

std::vector<std::vector<int>> minuteVectors = {
  
  {79,82,99}, //uur
  {12,29,32,49,  68,73,88,93}, //vijf over
  {52,69,72,89,  68,73,88,93}, //tien over
  
  {8,13,28,33,48,  68,73,88,93}, //kwart over
  {52,69,72,89,  14,27,34,47,  54,67,74,87}, //tien voor half
  {12,29,32,49,  14,27,34,47,  54,67,74,87}, //vijf voor half
  
  {54,67,74,87}, //half
  {12,29,32,49,  68,73,88,93,  54,67,74,87}, //vijf over half
  {52,69,72,89,  68,73,88,93,  54,67,74,87}, //tien over half
  
  {8,13,28,33,48,  14,27,34,47}, //kwart voor
  {52,69,72,89,    14,27,34,47}, //tien voor
  {12,29,32,49,    14,27,34,47}, //vijf voor
  
};

std::vector<std::vector<int>> looseMinuteVectors = {
  {},
  {40},
  {40,41},
  {40,41,60},
  {40,41,60,61},
};


Screen * screen;



void renderSpecificTime(int relevantHour, int minutes, int seconds){
  if(seconds == 0){
    screen->showSettings();
  }
//  Serial.println("renderSpecificTime()");

  Serial.print("Current time: ");
  Serial.print(relevantHour);
  Serial.print(":");
  Serial.println(minutes);

  screen->clear();

  
//  Serial.println("cleared the leds");

  /**/
  
  std::vector<int> staticLeds = {
    10,11,30,  50,51,  71,90   //het is nu
  };
  
  /*/
  
  std::vector<int> staticLeds = {
    1,2,3, 4, 5,6, 7, 8,9, 10,
    11,12,13, 14, 15,16, 17, 18,19, 20,
    21,22,23, 24, 25,26, 27, 28,29, 30,
    31,32,33, 34, 35,36, 37, 38,39, 40,
    41,42,43, 44, 45,46, 47, 48,49, 50,
    51,52,53, 54, 55,56, 57, 58,59, 60,
    61,62,63, 64, 65,66, 67, 68,69, 70,
    71,72,73, 74, 75,76, 77, 78,79, 80,
    81,82,83, 84, 85,86, 87, 88,89, 90,
    
    91,92,93, 94
  };

  /**/
  
  for(size_t i=0; i<staticLeds.size(); i++){
    int pos = staticLeds[i] - 1;
    screen->on(pos);
  }


  std::vector<int> hourLeds = hourVectors[relevantHour];
  
  for(size_t i=0; i<hourLeds.size(); i++){
    int pos = hourLeds[i] - 1;
//    Serial.print(pos);
    screen->on(pos);
  }
//  Serial.println("set the hour leds");

  int minuteIndex = minutes/5;
  std::vector<int> minuteLeds = minuteVectors[minuteIndex];

  for(size_t i=0; i<minuteLeds.size(); i++){
    int pos = minuteLeds[i] - 1;
//    Serial.print(pos);
    screen->on(pos);
  }
//  Serial.println("set the minute leds");

  int looseMinutes = minutes%5;
  std::vector<int> looseMinuteLeds = looseMinuteVectors[looseMinutes];

  for(size_t i=0; i<looseMinuteLeds.size(); i++){
    int pos = looseMinuteLeds[i] - 1;
//    Serial.print(pos);
    screen->on(pos);
  }
//  Serial.println("set the loose minute leds");


//  screen->debug();
  
}

bool enableRenderTime = true;

void renderTime(){
  if(!enableRenderTime){
    return;
  }

 Serial.println("renderTime()");
  
  int _hour = hour();
  int minutes = minute();

  int relevantHour = minutes < 20 ? _hour : _hour+1;

  while(relevantHour > 12){
    relevantHour -= 12;
  }

  renderSpecificTime(relevantHour, minutes, second());
  
 Serial.println("renderTime done");
}

int ticksSinceLastSecond = 0;
char lastSecond = -1;
void screenTick(){
  // auto start = micros();
  screen->tick();
  // auto duration = micros() - start;
  // Serial.printf("Duration: % 6d\n", duration);

  ticksSinceLastSecond++;
  if(second() != lastSecond){
    lastSecond = second();
    Serial.printf("% 3d fps\n", ticksSinceLastSecond);
    ticksSinceLastSecond = 0;
  }
}

Scheduler runner;

Task taskTime(5000, TASK_FOREVER, &renderTime);
Task taskTick(25, TASK_FOREVER, &screenTick);
Task taskNtp(1000, TASK_FOREVER, &Time_loop);


void setup() {
  wdt_disable();
 

  Serial.begin(115200);


  runner.init();
  Serial.println("Initialized scheduler");

  runner.addTask(taskTime);
  runner.addTask(taskTick);
  runner.addTask(taskNtp);

  taskTime.enable();
  taskTick.enable();
  taskNtp.enable();

  Serial.println("Scheduled tasks");


  screen = new Screen(NUM_LEDS, 128 * 1.0f);

  Wifi_setup();

  Time_setup();

  Serial.println("Clearing screen");
  screen->clear(true);
  
  Serial.println("Cleared screen");

  wdt_enable(5000);
}


void loop() {

  wdt_reset();
  
  // Time_loop();
  // MQTT_loop();
  // OTA_loop();
  // HTTP_loop();

  runner.execute();

  while(Serial.available()) {

    String command;
    command = Serial.readString();// read the incoming data as string

    Serial.print("Have command: ");
    Serial.println(command);

    int index = command.indexOf(":");
    if(index >= 0){
      String name = command.substring(0, index);
      Serial.print("Have command name: ");
      Serial.println(name);
      if(name.equals("t")){


        //Nov 07 2022 10:12:54
        //t:1667815974

        //Nov 07 2022 20:11:37
        //t:1667848297

        String timestamp = command.substring(index+1);
        int value = timestamp.toInt();
        setTime(value);
        Serial.print("Set timestamp value to: ");
        Serial.println(value);
      } else if(name.equals("r")) {
        screen->rainbow();
      } else if(name.equals("s")) {
        enableRenderTime = !enableRenderTime;
        screen->on();
      }
    }

  }

  

}
