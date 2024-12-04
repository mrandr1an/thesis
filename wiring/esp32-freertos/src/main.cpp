#include <Arduino.h>
#include "Task.cpp"

void setup()
{
  xTaskCreatePinnedToCore(TaskInitNetwork,"TaskNetwork",4096,NULL,1,&TaskNetwork_h,ARDUINO_RUNNING_CORE);
  xTaskCreatePinnedToCore(TaskInitHW,"TaskHW",4096,NULL,1,&TaskHW_h,ARDUINO_RUNNING_CORE);
  xTaskCreatePinnedToCore(TaskSenseDistance,"TaskHCSR04",4096,NULL,1,&TaskHCSR04_h,ARDUINO_RUNNING_CORE);
}

void loop()
{
}
