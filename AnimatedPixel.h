

#define FADE_IN_TIME_IN_SECONDS 5.0f
#define FADE_OUT_TIME_IN_SECONDS 5.0f


float brightness = 255.0f;
float hue_cycle_in_seconds = 120.0f;


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

    bool clearNext = false;

    uint8_t r = 0;
    uint8_t g = 0;
    uint8_t b = 0;

    Mode mode = constant;

    void tickBrightness(float timeFactor){
      
      if(this->clearNext){
          //this->hue = 0;
          //this->brightness = 0;
          //this->saturation = 0;
          this->targetBrightness = 0;
          this->mode = fade;
          this->clearNext = false;
      }

      if(this->brightness - this->targetBrightness > 0.01f){
          this->brightness = std::max(0.0f,this->brightness - (timeFactor * (brightness/FADE_OUT_TIME_IN_SECONDS)));
      }
      
      if(this->brightness - this->targetBrightness < -0.01f){
          this->brightness = std::min(255.0f,this->brightness + (timeFactor * (brightness/FADE_IN_TIME_IN_SECONDS)));
      }
      
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
    }
    


    void clear(){
      this->clearNext = true;
    }

    void setRainbow(){
      this->clearNext = false;
      //Serial.println("Setting pixel to rainbow.");
      if(this->mode != rainbow){
        Serial.println(this->mode);
        this->hue = random(0,256);    
        this->targetBrightness = brightness;
        this->saturation = 255;
        this->mode = rainbow;
      }else{
        //Serial.println("Already a rainbow");
      }
    }

};

