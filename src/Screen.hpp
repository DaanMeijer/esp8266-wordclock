// #define FASTLED_ESP8266_RAW_PIN_ORDER
#define FASTLED_ALLOW_INTERRUPTS 0

#define LED_TYPE WS2812B  // type of RGB LED's

#include <Adafruit_NeoPixel.h>

#include <Arduino.h>

#include "Pixel.hpp"

#include <vector>

#include "MQTT.hpp"


#define SCREEN_DATA_PIN  D2

class Screen {

  public: 
    enum Mode {
      RAINBOW,
      RANDOM,
      SYNCHRONOUS,
      JITTER,
    };


  private:
    size_t count;
    Adafruit_NeoPixel * screen;


    float brightness = 255.0f;
    float hue_cycle_in_seconds = 120.0f;

    long prevTick = micros();

    std::vector<Pixel*> pixels = {};

    Mode mode = Mode::RAINBOW;


  public:

//    static String modeString[] = { 
//      "Rainbow", 
//      "Random", 
//      "Mono", 
//      "Jitter" 
//    };


    Screen(size_t count, float brightness) {
      this->count = count;
      this->brightness = brightness;

      this->screen = new Adafruit_NeoPixel(count, SCREEN_DATA_PIN, NEO_RGB + NEO_KHZ800);


      this->screen->begin();           // INITIALIZE NeoPixel strip object (REQUIRED)
      this->screen->show();            // Turn OFF all pixels ASAP

      this->init(this->count);

      this->setBrightness(this->brightness);
    }

    void setHueCycleInSeconds(float cycle) {
      for (size_t i = 0; i < pixels.size(); i++) {
        pixels[i]->hue_cycle_in_seconds = cycle;
      }
    }

    void setBrightness(float brightness) {

      Serial.printf("setBrightness(%f)\n", brightness);
      this->brightness = brightness;

      auto adjusted = 0;
      for (size_t i = 0; i < pixels.size(); i++) {
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


    void init(size_t count) {

      uint8_t brightness = this->brightness * 1;
      screen->setBrightness(brightness);


      uint8_t initHue = random(0, 255);
      for (size_t i = 0; i < this->count; i++) {
        Pixel * pixel = new Pixel();
        pixel->position = i;
        pixel->hue = initHue;
        pixel->saturation = 255;
        pixel->brightness = this->brightness;
        pixel->mode = Pixel::Mode::rainbow;

        pixels.push_back(pixel);

        if(i%10==9){
          initHue++;
        }

      }

    }

    void render() {
      
      for (size_t i = 0; i < pixels.size(); i++) {

        Pixel * pixel = pixels[i];

        switch(pixel->colorMode){
          case Pixel::ColorMode::COLOR_RGB:
            if(pixel->brightness < 0.01f){
              screen->setPixelColor(i, screen->Color(0, 0, 0));
            }else{
              screen->setPixelColor(i, screen->Color(pixel->r, pixel->g, pixel->b));
            }
            break;

          case Pixel::ColorMode::COLOR_HSV:
            uint8_t brightness = pixel->getBrightness();

            this->screen->setPixelColor(i, /*screen->gamma32*/(screen->ColorHSV(pixel->hue * 256.0f, pixel->saturation, brightness)));
            break;  
        }

        // if(pixel->brightness > 0.01f && i == 9){   
            // Serial.printf("led[%03d]: %06x pixel %02x %02x %02x\n", i, screen->getPixelColor(i), (uint8_t)pixel->hue, (uint8_t)pixel->saturation, (uint8_t)pixel->brightness);
        // }


      }

    // //   Serial.println();
    //   for (size_t i = 0; i < pixels.size(); i++) {
    //     leds[i].setRGB(i==0?255:0, 0, 0);
    //   }


      screen->show();

    }
    
    void debug(){
      Serial.printf("Screen:\nbrightness: %f\n", this->brightness);
      //pixel 9 is always on (H)
      //pixels[9]->debug();

      pixels[0]->debug();
    }


    void tick() {

      long difference = micros() - this->prevTick;
      this->prevTick = micros();

      difference = min(difference, 500000L);

      float timeFactor = difference / 1000000.0f;

      // Serial.println("Calculating pixels");
      for (size_t i = 0; i < pixels.size(); i++) {
        pixels[i]->tick(timeFactor);
      }

      // Serial.println("Rendering...");
      this->render();
      // Serial.println("Rendered");

    }

    void clear() {
      this->clear(false);
    }
    
    void clear(bool force) {
      
//      Serial.println(force ? "Clearing screen forced" : "Clearing screen");
      for (size_t i = 0; i < this->count; i++) {
        pixels[i]->clear(force);
      }
    }

    void mono() {
      
      this->mode = Screen::Mode::SYNCHRONOUS;
      uint8_t hue = random(0, 255);
      this->setHSV(hue, 255, this->brightness);
      this->hue_cycle_in_seconds = 0.0f;
      for (size_t i = 0; i < this->count; i++) {
        pixels[i]->hue_cycle_in_seconds = this->hue_cycle_in_seconds;
      }
      this->showSettings();
    }

    void colorMode(Pixel::ColorMode mode){
      for (size_t i = 0; i < this->count; i++) {
        pixels[i]->colorMode = mode;
      }
    }
    

    void synchronize() {
      this->mode = Screen::Mode::SYNCHRONOUS;
      uint8_t hue = random(0, 255);
      this->setHSV(hue, 255, this->brightness);
      this->hue_cycle_in_seconds = 120.0f;
      for (size_t i = 0; i < this->count; i++) {
        pixels[i]->hue_cycle_in_seconds = this->hue_cycle_in_seconds;
      }
      this->showSettings();
    }

    void setHSV(float hue, float saturation, float brightness){
      for (size_t i = 0; i < this->count; i++) {
        pixels[i]->hue = hue;
        pixels[i]->saturation = saturation;
      }
      this->colorMode(Pixel::ColorMode::COLOR_HSV);
      this->setBrightness(brightness);
    }

    void setRGB(uint8_t r, uint8_t g, uint8_t b){
      for (size_t i = 0; i < this->count; i++) {
        pixels[i]->r = r;
        pixels[i]->g = g;
        pixels[i]->b = b;
      }
      this->colorMode(Pixel::ColorMode::COLOR_RGB);
    }

    void randomize() {
      this->mode = Screen::Mode::RANDOM;
      this->colorMode(Pixel::ColorMode::COLOR_HSV);
      for (size_t i = 0; i < this->count; i++) {
        uint8_t hue = random(0, 255);
        pixels[i]->hue = hue;
      }
      this->showSettings();
    }

    void jitterize() {
      this->mode = Screen::Mode::JITTER;
      this->colorMode(Pixel::ColorMode::COLOR_HSV);
      for (size_t i = 0; i < this->count; i++) {
        float interval = random(2, this->hue_cycle_in_seconds);
        pixels[i]->hue_cycle_in_seconds = interval;
      }
      this->showSettings();
    }

    void gradientize() {
      this->synchronize();
      this->mode = Screen::Mode::RAINBOW;
      this->colorMode(Pixel::ColorMode::COLOR_HSV);
      for (size_t i = 0; i < this->count; i++) {
        pixels[i]->hue += i;
      }
      this->showSettings();
    }

    void rainbow() {
      this->mode = Screen::Mode::RAINBOW;
      this->colorMode(Pixel::ColorMode::COLOR_HSV);
      for (size_t i = 0; i < pixels.size(); i++) {
        Pixel * pixel = pixels[i];
        pixel->setRainbow(this->brightness);
      }
    }

    void on() {
      for (size_t i = 0; i < pixels.size(); i++) {
        this->on(i, false);
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
