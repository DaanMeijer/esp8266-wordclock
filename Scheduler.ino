
#include <vector>

typedef struct intervalFunction {
  int intervalInMs;
  int prevExecution;
  void (*function)();
} intervalFunction;

std::vector<intervalFunction*> intervalFunctions;


void addFunction(void (*function)(), int intervalInMs){
  
  intervalFunction * func = (intervalFunction *)malloc(sizeof *func) ;
  
  func->function = function;
  func->intervalInMs = intervalInMs;
  func->prevExecution = 0;

  intervalFunctions.push_back(func);
}

void Scheduler_loop(){

  #if DEBUG_GENERAL
  Serial.println("Scheduler_loop()");
  #endif
  for(int i=0; i<intervalFunctions.size(); i++){
    intervalFunction *func = intervalFunctions[i];

    int currentTime = millis();
    
    int millisSinceLastExec = currentTime - func->prevExecution;

#if DEBUG_SCHEDULER
    Serial.print(currentTime);
    Serial.print(", last execution ");
    Serial.print(func->prevExecution);
    Serial.print(", interval ");
    Serial.println(func->intervalInMs);
#endif
    
    if(func->intervalInMs < millisSinceLastExec){
#if DEBUG_SCHEDULER
      Serial.println("Executing...");
#endif
      func->prevExecution = currentTime;
      func->function();
    }
    
  }
}
