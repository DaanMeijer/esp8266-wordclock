
#define FASTLED_ESP8266_RAW_PIN_ORDER
#define FASTLED_ALLOW_INTERRUPTS 0

#include "FastLED.h"

#include "AnimatedPixel.h"

#include <vector>

enum ColorMode {
  Rainbow,
  OneRainbow,
  Constant
};

#define COLOR_MODE ColorMode::Rainbow
#define CONSTANT_COLOR 0x40E0D0 //turquoise

class AnimatedPixels {

  private:
    int count;
    CRGB * leds;

        
    float brightness = 255.0f;
    float hue_cycle_in_seconds = 120.0f;
    
    long prevTick = micros();
    
    std::vector<AnimatedPixel*> pixels = {};
  
  public:
  
    AnimatedPixels(int count){
      this->count = count;

      this->leds = new CRGB[count];
      
      this->init(this->count);

      this->setBrightness(this->brightness);
    }

    void setHueCycleInSeconds(float cycle){
      for(int i=0; i < pixels.size(); i++){
        pixels[i]->hue_cycle_in_seconds = cycle;
      }
    }

    void setBrightness(float brightness){
      
      Serial.printf("setBrightness(%f)\n", brightness);
      this->brightness = brightness;

      
      
      for(int i=0; i < pixels.size(); i++){
        if(pixels[i]->targetBrightness > 0.1f){
          pixels[i]->setTargetBrightness(brightness);
        }
      }

    }
      
    void rainbow(){
      for(int i=0; i < pixels.size(); i++){
        AnimatedPixel * pixel = pixels[i];
        pixel->setRainbow(this->brightness);
      }
    }
     
    
    void init(int count){
    
      LEDS.addLeds<NEOPIXEL, SCREEN_DATA_PIN>(this->leds, count);
      
      byte brightness = this->brightness * 1;
      LEDS.setBrightness(brightness);
      

      byte initHue = random(0,255);
      for(int i=0; i<this->count; i++){
        AnimatedPixel * pixel = new AnimatedPixel();
        pixel->position = i;
        pixel->hue = initHue;
        pixel->saturation = 255;
        pixel->brightness = this->brightness;
        pixel->mode = AnimatedPixel::Mode::rainbow;
        
        pixels.push_back(pixel);
        
        initHue++;
        
      }
    
      this->render();
    
    }
    
    void render(){
//      Serial.printf("AnimatedPixels brightness: %f\n", this->brightness);   
      FastLED.clear();
//      Serial.println("renderAnimatedPixels");
//      pixels[3]->debug();
//      Serial.println("brightness: ");
      for(int i=0; i < pixels.size(); i++){
        
        AnimatedPixel * pixel = pixels[i];
        byte brightness = pixel->getBrightness();
//        brightness = this->brightness;
//        Serial.printf("%02x ", brightness);
//        if(i%10 == 9){
//          Serial.println("");
//        }
        
          int iHue = pixels[i]->hue;
          iHue = iHue % 256;
          uint8_t hue = iHue;
          leds[pixel->position].setHSV(hue, pixel->saturation, brightness);
    
        

      }
      
//      Serial.println("showing fastled");
      
      FastLED.show();
//      Serial.println("renderAnimatedPixels done");
    }
   

    void tick(){
    
      long difference = micros() - this->prevTick;
      this->prevTick = micros();
    
      float timeFactor = difference / 1000000.0f;
    
      for(int i=0; i < pixels.size(); i++){
        pixels[i]->tick(timeFactor);
      }
      
      this->render();
      
    }
    
    void clear(){
      for(int i=0; i<this->count; i++){
        pixels[i]->clear();
      }
    }

    void synchronize(){
      byte hue = random(0,255);
      for(int i=0; i<this->count; i++){
        pixels[i]->hue = hue;
        pixels[i]->hue_cycle_in_seconds = this->hue_cycle_in_seconds;
      }
    }

    void randomize(){
      for(int i=0; i<this->count; i++){
        byte hue = random(0,255);
        pixels[i]->hue = hue;
      }
    }

    void jitterize(){
      for(int i=0; i<this->count; i++){
        float interval = random(2,this->hue_cycle_in_seconds);
        pixels[i]->hue_cycle_in_seconds = interval;
      }
    }

    void gradientize(){
      this->synchronize();
      for(int i=0; i<this->count; i++){
        pixels[i]->hue += i;
      }
    }
    
    void on(int pos){
      pixels[pos]->setTargetBrightness(this->brightness);
    }
};
