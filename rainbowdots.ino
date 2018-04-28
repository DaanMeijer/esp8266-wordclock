//#include <TimeLib.h>
#include <Time.h>

#define FASTLED_ESP8266_RAW_PIN_ORDER
#define FASTLED_ALLOW_INTERRUPTS 0

#include "FastLED.h"


//254
// 1e letter 49
// laatste letter eindigt 208
//229
// 1e letter 24
// laatste 187


#include <TimeLib.h>
#include <vector>


#include <ESP8266WiFi.h>
#include <ESP8266mDNS.h>
#include <WiFiManager.h>

#include <ArduinoOTA.h>




#define DEBUG_SCHEDULER 0
#define DEBUG_LEDS 0

#define NUM_LEDS        95
#define DATA_PIN        D1 // D2 Pin on Wemos mini


CRGB leds[NUM_LEDS];

#include "AnimatedPixel.h"

typedef struct intervalFunction {
  int intervalInMs;
  int prevExecution;
  void (*function)();
} intervalFunction;

std::vector<intervalFunction*> intervalFunctions;


typedef struct rainbowPosition {
  uint8_t position;
  double hue;
  double hueDelta;
  double saturation;
  double saturationDelta;
  double brightness;
  double brightnessDelta;
} rainbowPosition;



std::vector<rainbowPosition> rainbowPositions = {
};


uint8_t gHue = 0;
uint8_t hueDelta = 1;

void initRainbow(){
  for(int i=0; i<NUM_LEDS; i++){
    rainbowPosition pos;
    pos.position = i;
    pos.hue = map(i, 0, NUM_LEDS, 0, 255);
    pos.hueDelta = 0;
    pos.saturation = 255;
    pos.saturationDelta = 0;
    pos.brightness = 255;
    pos.brightness = -0.05d;

    rainbowPositions.push_back(pos);
  }
}
    
void rainbowTick(){
  for(int i=0; i<rainbowPositions.size(); i++){
    rainbowPosition * led = &rainbowPositions[i];

    led->hue += led->hueDelta;
    
    led->saturation = std::max(0.0d, std::min(255.0d, led->saturation + led->saturationDelta));
    
    led->brightness = std::max(0.0d, std::min(255.0d, led->brightness + led->brightnessDelta));
    
    
    leds[led->position].setHSV(led->hue, led->saturation, led->brightness);
#if DEBUG_LEDS
    Serial.print("setting led ");
    Serial.print(led->position);
    Serial.print(" to hue ");
    Serial.println(led->currentHue);
#endif
  }
  FastLED.show();
}


void addFunction(void (*function)(), int intervalInMs);
void runFunctions();


std::vector<AnimatedPixel*> pixels = {};

void initAnimatedPixels(){
  for(int i=0; i<NUM_LEDS; i++){
    AnimatedPixel * pixel = new AnimatedPixel();
    pixel->position = i;
    pixel->hue = 0;
    pixel->saturation = 0;
    pixel->brightness = 100;

    pixels.push_back(pixel);
  }
}

long animatedPixelTickPrevMicros = micros();

void animatedPixelTick(){

  long difference = micros() - animatedPixelTickPrevMicros;
  animatedPixelTickPrevMicros = micros();

  float timeFactor = difference / 1000000.0f;

  for(int i=0; i < pixels.size(); i++){
    pixels[i]->tick(timeFactor);
  }
  
}

void renderAnimatedPixels(){
  //Serial.println("renderAnimatedPixels");
  for(int i=0; i < pixels.size(); i++){
    AnimatedPixel * pixel = pixels[i];

    int iHue = pixel->hue;
    iHue = iHue % 256;
    uint8_t hue = iHue;
    leds[pixel->position].setHSV(hue, pixel->saturation, pixel->brightness);

/*
    Serial.println("Pixel");
    Serial.println(pixel->position);
    Serial.println(pixel->mode);
    Serial.println(hue);
    Serial.println(pixel->saturation);
    Serial.println(pixel->brightness);
    Serial.println();
*/
  }
  
  FastLED.show();
}

void timeTick(){
  time_t t = now();
  int size = rainbowPositions.size();
  rainbowPositions[t % size].hueDelta = 0;
  rainbowPositions[(t - 1) % size].hueDelta = 1;
}

void secondsPulse(){
  
  for(int i=0; i<rainbowPositions.size(); i++){
    rainbowPositions[i].brightness = 255;
    rainbowPositions[i].brightnessDelta = -0.1d;
  }
  
}



void doOta();
void startWiFi();

void setupNtp();


std::vector<std::vector<int>> hourVectors = {
  {}, //0, skip het
  {66,75,86}, //een
  {74,75,76,77}, //twee
  {41,42,43,44}, //drie
  {60,59,58,57}, //vier
  {82,83,84,85}, //vijf
  {71,72,73}, //zes
  {61,62,63,64,65}, //zeven
  {77,78,79,80}, //acht
  {65,66,67,68,69}, //negen
  {47,48,49,50}, //tien
  {44,45,46}, //elf
  {51,52,53,54,55,56}, //twaalf
};

std::vector<std::vector<int>> minuteVectors = {
  
  {88,89,90}, //uur
  {19,18,17,16,  27,28,29,30}, //vijf over
  {15,14,13,12,  27,28,29,30}, //tien over
  
  {21,22,23,24,25,  27,28,29,30}, //kwart over
  {15,14,13,12,  40,39,38,37,  35,34,33,32}, //tien voor half
  {19,18,17,16,  40,39,38,37,  35,34,33,32}, //vijf voor half
  
  {35,34,33,32}, //half
  {19,18,17,16,  27,28,29,30,  35,34,33,32}, //vijf over half
  {15,14,13,12,  27,28,29,30,  35,34,33,32}, //tien over half
  
  {21,22,23,24,25,  40,39,38,37}, //kwart voor
  {15,14,13,12,  40,39,38,37}, //tien voor
  {19,18,17,16,  40,39,38,37}, //vijf voor
  
};



void renderSpecificTime(int relevantHour, int minutes){
  

  //Serial.print("Current time: ");
  //Serial.print(hour);
  //Serial.print(":");
  //Serial.println(minutes);

  for(int i=0; i<NUM_LEDS; i++){
    pixels[i]->clear();
  }

  
  //Serial.println("cleared the leds");

  std::vector<int> staticLeds = {
    1,2,3,  5,6,  8,9
  };
  
  for(int i=0; i<staticLeds.size(); i++){
    int pos = staticLeds[i] - 1;
    pixels[pos]->setRainbow();
  }


  std::vector<int> hourLeds = hourVectors[relevantHour];
  
  for(int i=0; i<hourLeds.size(); i++){
    int pos = hourLeds[i] - 1;
    pixels[pos]->setRainbow();
  }
  //Serial.println("set the hour leds");

  int minuteIndex = minutes/5;
  std::vector<int> minuteLeds = minuteVectors[minuteIndex];

  for(int i=0; i<minuteLeds.size(); i++){
    int pos = minuteLeds[i] - 1;
    pixels[pos]->setRainbow();
  }
  //Serial.println("set the minute leds");

  for(int i=0; i<=minutes%5; i++){
    int pos = 94 - i;
    //Serial.print("setting dot ");
    //Serial.println(pos);
    pixels[pos]->setRainbow();
  }

  //Serial.print("seconds: ");
  //Serial.println(second());
  
}


void renderTime(){
  int hour = hourFormat12();
  int minutes = minute();

  int relevantHour = minutes < 20 ? hour : hour+1;

  if(relevantHour > 12){
    relevantHour -= 12;
  }

  renderSpecificTime(relevantHour, minutes);
}

int currentDemoHour = 0;
void runDemo(){
  currentDemoHour = (currentDemoHour + 1) % 12; 
  renderSpecificTime(currentDemoHour + 1,0);
}

void setup() {


    randomSeed(analogRead(0));

    Serial.begin(115200);
    
    LEDS.addLeds<NEOPIXEL,DATA_PIN>(leds,NUM_LEDS);
    
    FastLED.clear();

    LEDS.setBrightness(50);
    FastLED.show();

    //initRainbow();
    //addFunction(rainbowTick, 1);
    
    //addFunction(timeTick, 1000);
    //addFunction(secondsPulse, 4000);

    initAnimatedPixels();
    
    addFunction(animatedPixelTick, 1);
    addFunction(renderAnimatedPixels, 10);

    //addFunction(runDemo, 5000);
    
    addFunction(renderTime, 1000);
    
    startWiFi();
    doOta();

    setupNtp();
    return;
    MQTT_init("wordclock", callback);
    Serial.println("init MQTT");
    
    MQTT_publish("Hello Wordclock!");
    Serial.println("MQTT message sent");
    
}

// the loop function runs over and over again forever
void loop() {
  Serial.println("Looping...");
  OTA_loop();
  //MQTT_loop();
  
  runFunctions();

}



unsigned int localPort = 8888;

WiFiUDP     Udp;

void startWiFi(){
  
    WiFiManager wifiManager;

    String ssid = "WordClock-" + String(ESP.getChipId());
    wifiManager.autoConnect(ssid.c_str());
  
    Serial.println("Connected!");
    Serial.print("IP address: ");
    Serial.println(WiFi.localIP());
    Serial.println("Starting UDP");
    Udp.begin(localPort);
}



// Check if Daylight saving time (DST) applies
// Northern Hemisphere - +1 hour between March and October
bool isDST(int d, int m, int y){
    bool dst = false;
    dst = (m > 3 && m < 10); // October-March

    if (m == 3){
        // Last sunday of March
        dst = (d >= ((31 - (5 * y /4 + 4) % 7)));
    }else if (m == 10){
        // Last sunday of October
        dst = (d < ((31 - (5 * y /4 + 1) % 7)));
    }

    return dst;
}

bool isDSTSwitchDay(int d, int m, int y){
    bool dst = false;
    if (m == 3){
        // Last sunday of March
        dst = (d == ((31 - (5 * y /4 + 4) % 7)));
    }else if (m == 10){
        // Last sunday of October
        dst = (d == ((31 - (5 * y /4 + 1) % 7)));
    }
    return dst;
}



