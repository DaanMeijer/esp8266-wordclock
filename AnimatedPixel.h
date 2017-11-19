
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

    Mode mode = constant;

    
    
    void tick(float timeFactor){

      if(this->clearNext){
          //this->hue = 0;
          //this->brightness = 0;
          //this->saturation = 0;
          this->targetBrightness = 0;
          this->mode = fade;
          this->clearNext = false;
      }

      if(this->brightness - this->targetBrightness > 0.01f){
          this->brightness = std::max(0.0f,this->brightness - (timeFactor * (256.0f/5.0f)));
      }
      
      if(this->brightness - this->targetBrightness < -0.01f){
          this->brightness = std::min(255.0f,this->brightness + (timeFactor * (256.0f/5.0f)));
      }
      
      switch(this->mode){
        case rainbow:
          this->hue += (timeFactor * (256.0f/4.5f));
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
        this->targetBrightness = 255;
        this->saturation = 255;
        this->mode = rainbow;
      }else{
        //Serial.println("Already a rainbow");
      }
    }

};

