#define FASTLED_ESP8266_RAW_PIN_ORDER
#define FASTLED_ALLOW_INTERRUPTS 0

#include "FastLED.h"

#include "AnimatedPixel.h"

#include <vector>

#include "MQTT.h"

class AnimatedPixels {

  public: 
    enum Mode {
      RAINBOW,
      RANDOM,
      SYNCHRONOUS,
      JITTER,
    };


  private:
    int count;
    CRGB * leds;


    float brightness = 255.0f;
    float hue_cycle_in_seconds = 120.0f;

    long prevTick = micros();

    std::vector<AnimatedPixel*> pixels = {};

    Mode mode = Mode::RAINBOW;
    

  public:

//    static String modeString[] = { 
//      "Rainbow", 
//      "Random", 
//      "Mono", 
//      "Jitter" 
//    };

    AnimatedPixels(int count) {
      this->count = count;

      this->leds = new CRGB[count];

      this->init(this->count);

      this->setBrightness(this->brightness);
    }

    void setHueCycleInSeconds(float cycle) {
      for (int i = 0; i < pixels.size(); i++) {
        pixels[i]->hue_cycle_in_seconds = cycle;
      }
    }

    void setBrightness(float brightness) {

      Serial.printf("setBrightness(%f)\n", brightness);
      this->brightness = brightness;

      auto adjusted = 0;
      for (int i = 0; i < pixels.size(); i++) {
        if (pixels[i]->targetBrightness > 0.1f) {
          adjusted++;
          pixels[i]->setTargetBrightness(brightness);
        }
      }

      Serial.printf("adjusted: %d\n", adjusted);

      this->showSettings();

    }

    void showSettings() {
      char buff[255];
      
      sprintf(buff, "{\"POWER\":\"ON\",\"Dimmer\":%f,\"Mode\":\"%d\"}", this->brightness, this->mode);
      MQTT_publish("stat/WordClock/RESULT", buff);
    }


    void init(int count) {

      LEDS.addLeds<NEOPIXEL, SCREEN_DATA_PIN>(this->leds, count);

      byte brightness = this->brightness * 1;
      LEDS.setBrightness(brightness);


      byte initHue = random(0, 255);
      for (int i = 0; i < this->count; i++) {
        AnimatedPixel * pixel = new AnimatedPixel();
        pixel->position = i;
        pixel->hue = initHue;
        pixel->saturation = 255;
        pixel->brightness = this->brightness;
        pixel->mode = AnimatedPixel::Mode::rainbow;

        pixels.push_back(pixel);

        initHue++;

      }

    }

    void render() {
      
      for (int i = 0; i < pixels.size(); i++) {

        AnimatedPixel * pixel = pixels[i];

        switch(pixel->colorMode){
          case AnimatedPixel::ColorMode::COLOR_RGB:
            if(pixel->brightness < 0.01f){
              leds[pixel->position].setRGB(0, 0, 0);
            }else{
              leds[pixel->position].setRGB(pixel->r, pixel->g, pixel->b);
            }
            break;

          case AnimatedPixel::ColorMode::COLOR_HSV:
            byte brightness = pixel->getBrightness();

//          int iHue = pixels[i]->hue % 256;
            uint8_t hue = pixels[i]->hue;
            leds[pixel->position].setHSV(hue, pixel->saturation, brightness);
            break;  
        }


      }

      FastLED.show();
      
    }


    void tick() {

      long difference = micros() - this->prevTick;
      this->prevTick = micros();

      float timeFactor = difference / 1000000.0f;

      for (int i = 0; i < pixels.size(); i++) {
        pixels[i]->tick(timeFactor);
      }

      this->render();

    }

    void clear() {
      for (int i = 0; i < this->count; i++) {
        pixels[i]->clear();
      }
    }

    void mono() {
      
      this->mode = AnimatedPixels::Mode::SYNCHRONOUS;
      byte hue = random(0, 255);
      this->setHSV(hue, 255, this->brightness);
      this->hue_cycle_in_seconds = 0.0f;
      for (int i = 0; i < this->count; i++) {
        pixels[i]->hue_cycle_in_seconds = this->hue_cycle_in_seconds;
      }
      this->showSettings();
    }

    void colorMode(AnimatedPixel::ColorMode mode){
      for (int i = 0; i < this->count; i++) {
        pixels[i]->colorMode = mode;
      }
    }
    

    void synchronize() {
      this->mode = AnimatedPixels::Mode::SYNCHRONOUS;
      byte hue = random(0, 255);
      this->setHSV(hue, 255, this->brightness);
      this->hue_cycle_in_seconds = 120.0f;
      for (int i = 0; i < this->count; i++) {
        pixels[i]->hue_cycle_in_seconds = this->hue_cycle_in_seconds;
      }
      this->showSettings();
    }

    void setHSV(float hue, float saturation, float brightness){
      for (int i = 0; i < this->count; i++) {
        pixels[i]->hue = hue;
        pixels[i]->saturation = saturation;
      }
      this->colorMode(AnimatedPixel::ColorMode::COLOR_HSV);
      this->setBrightness(brightness);
    }

    void setRGB(byte r, byte g, byte b){
      for (int i = 0; i < this->count; i++) {
        pixels[i]->r = r;
        pixels[i]->g = g;
        pixels[i]->b = b;
      }
      this->colorMode(AnimatedPixel::ColorMode::COLOR_RGB);
    }

    void randomize() {
      this->mode = AnimatedPixels::Mode::RANDOM;
      this->colorMode(AnimatedPixel::ColorMode::COLOR_HSV);
      for (int i = 0; i < this->count; i++) {
        byte hue = random(0, 255);
        pixels[i]->hue = hue;
      }
      this->showSettings();
    }

    void jitterize() {
      this->mode = AnimatedPixels::Mode::JITTER;
      this->colorMode(AnimatedPixel::ColorMode::COLOR_HSV);
      for (int i = 0; i < this->count; i++) {
        float interval = random(2, this->hue_cycle_in_seconds);
        pixels[i]->hue_cycle_in_seconds = interval;
      }
      this->showSettings();
    }

    void gradientize() {
      this->synchronize();
      this->mode = AnimatedPixels::Mode::RAINBOW;
      this->colorMode(AnimatedPixel::ColorMode::COLOR_HSV);
      for (int i = 0; i < this->count; i++) {
        pixels[i]->hue += i;
      }
      this->showSettings();
    }

    void rainbow() {
      this->mode = AnimatedPixels::Mode::RAINBOW;
      this->colorMode(AnimatedPixel::ColorMode::COLOR_HSV);
      for (int i = 0; i < pixels.size(); i++) {
        AnimatedPixel * pixel = pixels[i];
        pixel->setRainbow(this->brightness);
      }
    }

    void on(int pos) {
      this->on(pos, false);
    }
    void on(int pos, bool force) {
      pixels[pos]->setTargetBrightness(this->brightness);
      if(force){
        pixels[pos]->brightness = this->brightness;
      }
    }
};
