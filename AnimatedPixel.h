#define FADE_IN_TIME_IN_SECONDS 5.0f
#define FADE_OUT_TIME_IN_SECONDS 5.0f

class AnimatedPixel {

  public:
    
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

    
    float hue_cycle_in_seconds = 120.0f;

    bool clearNext = false;

    uint8_t r = 0;
    uint8_t g = 0;
    uint8_t b = 0;
    
    float maxBrightness = 255.0f;

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
          this->brightness = std::max(0.0f,this->brightness - (timeFactor * (maxBrightness/FADE_OUT_TIME_IN_SECONDS)));
      }
      
      if(this->brightness - this->targetBrightness < -0.01f){
          this->brightness = std::min(255.0f,this->brightness + (timeFactor * (maxBrightness/FADE_IN_TIME_IN_SECONDS)));
      }
      
    }

    byte getBrightness(){
//      byte result = 1*this->brightness;
      
      byte result = map(this->brightness, 0, 255, 0, 255);
      
//      Serial.printf("getBrightness(%f): %d\n", this->brightness, result);
      return result;
      return 128;
      return 1 * this->brightness; //map(this->brightness, 0, 255, 0, 255);
    }
    
    void tick(float timeFactor){

      
      switch(this->mode){
        case rainbow:
          this->hue += (timeFactor * (256.0f/hue_cycle_in_seconds));
          break;

        case fade:


        case constant:
          
          break;
      }

      this->tickBrightness(timeFactor);
    }
    


    void clear(){
      this->clearNext = true;
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
      Serial.println("");
    }

};

