#define FADE_IN_TIME_IN_SECONDS 5.0f
#define FADE_OUT_TIME_IN_SECONDS 5.0f


#include <Arduino.h>

class Pixel {

  public:
    enum ColorMode{
      COLOR_RGB,
      COLOR_HSV
    };
    
    enum Mode {
      rainbow,
      fade,
      constant,
      clearMe
    };
  
    int position;

    float hue = 0;
    float brightness = 0;
    float saturation = 0;
    float targetBrightness = 0;

    ColorMode colorMode = COLOR_HSV;
    
    float hue_cycle_in_seconds = 120.0f;

    bool clearNext = false;

    uint8_t r = 0;
    uint8_t g = 0;
    uint8_t b = 0;
    
    float maxBrightness = 255.0f;

    float fadeInTime = 5.0f;
    float fadeOutTime = 5.0f;

    Mode mode = constant;

    void tickBrightness(float timeFactor){
      
      if(this->clearNext){
          //this->hue = 0;
          //this->brightness = 0;
          //this->saturation = 0;
          this->targetBrightness = 0;
//          this->mode = fade;
          this->clearNext = false;
      }

      if(this->brightness - this->targetBrightness > 0.01f){
          this->brightness = std::max(0.0f,this->brightness - (timeFactor * (maxBrightness/this->fadeOutTime)));
      }
      
      if(this->brightness - this->targetBrightness < -0.01f){
          this->brightness = std::min(255.0f,this->brightness + (timeFactor * (maxBrightness/this->fadeInTime)));
      }
      
    }

    uint8_t getBrightness(){
      uint8_t result = map(this->brightness, 0, 255, 0, 255);
      return result;
    }
    
    void tick(float timeFactor){

      
      switch(this->mode){
        case rainbow:
          if(hue_cycle_in_seconds != 0){
            this->hue += (timeFactor * (256.0f/hue_cycle_in_seconds));
          }
          break;

        case fade:


        case constant:
        case clearMe:
          
          break;
      }

      this->tickBrightness(timeFactor);
    }

    void clear(){
      this->clear(false);
    }

    void clear(bool force){
      if(force){
        this->brightness = 0.0f;
      }else{
        this->clearNext = true;
      }
    }

    void setTargetBrightness(float brightness){
      if(brightness > 0.01f){
        this->clearNext = false;
      }
      this->maxBrightness = brightness;
      this->targetBrightness = brightness;
    }

    void setRainbow(float brightness){
//      Serial.println("Setting pixel to rainbow.");
      if(this->mode != rainbow){
        Serial.println(this->mode);
        this->hue = random(0,256);    
        this->saturation = 255;
        this->mode = rainbow;
      }else{
//        Serial.println("Already a rainbow");
      }
      
      this->setTargetBrightness(brightness);
    }
    
    void debug(){
      Serial.println("Pixel:");
      Serial.printf("hue: %f\n", this->hue);
      Serial.printf("saturation: %f\n", this->saturation);
      Serial.printf("brightness: %f\n", this->brightness);
      Serial.printf("target brightness: %f\n", this->targetBrightness);

      
      Serial.printf("color mode: %d\n", this->colorMode);
      Serial.printf("hue cycle: %f\n", this->hue_cycle_in_seconds);
      Serial.printf("clear next: %s\n", this->clearNext ? "yes" : "no");
      Serial.printf("hex: %02x%02x%02x\n", this->r, this->g, this->b);
      Serial.printf("max brightness: %f\n", this->maxBrightness);
      Serial.printf("mode: %d\n", this->mode);
      

      
      Serial.println("");
    }

};
