#include <Arduino.h>
#include <WiFi.h>
#include <PubSubClient.h>
#include "Config.hpp"

#define DEBUG

#if CONFIG_FREERTOS_UNICORE
#define ARDUINO_RUNNING_CORE 0
#else
#define ARDUINO_RUNNING_CORE 1
#endif
#define SYSTEM_RUNNING_CORE 0

#define GREEN 2
#define BLUE 16
#define RED 18
#define BUTTON 19

TaskHandle_t TaskNetwork_h;

void TaskSense(void *pvParameters);

void TaskPublish(void *pvParameters);

void TaskListen(void *pvParameters);

void TaskKeepAlive(void *pvParameters);

WiFiClient wifiClient;
PubSubClient mqttClient(wifiClient);
void TaskInitNetwork(void *pvParameters)
{
  //BEGIN INIT WIFI

  //Set wifi to station mode and set name & pwd
  WiFi.mode(WIFI_STA);
  WiFi.begin(WIFI_NAME,WIFI_PASS);

  int timeout = 0;
  while (timeout < WIFI_TIMEOUT_SECONDS)
  {
    if (WiFi.status() != WL_CONNECTED)
    {
      #ifdef DEBUG
      Serial.print("Wifi not connected yet with status: ");
      Serial.println(WiFi.status());
      #endif //DEBUG
      vTaskDelay(pdMS_TO_TICKS(1000));
      timeout++;
    } else
    {
      #ifdef DEBUG
      Serial.print("Wifi connected in network: ");
      Serial.print(WiFi.SSID());
      Serial.print(" with RSSI: ");
      Serial.print(WiFi.RSSI());
      Serial.print(" and IP: ");
      Serial.println(WiFi.localIP());
      #endif //DEBUG
      break;
    }
  }

  //END INIT WIFI

  //BEGIN INIT MQTT

  mqttClient.setServer(MQTT_BROKER_IP,MQTT_BROKER_PORT);
  timeout = 0;
  while (timeout < MQTT_TIMEOUT_SECONDS) {
    if (mqttClient.state() != MQTT_CONNECTED)
    {
      #ifdef DEBUG
      Serial.print("MQTT not connected yet with status: ");
      Serial.println(mqttClient.state());
      #endif //DEBUG
      vTaskDelay(pdMS_TO_TICKS(1000));
      timeout++;
    } else
    {
    #ifdef DEBUG
    Serial.println("MQTT connected to broker");
    #endif //DEBUG
    }
  }

  //END INIT MQTT

  //Delete task after initialization
  vTaskDelete(NULL);
}

void TaskInitHW(void *pvParameters)
{ 
  //Delete task after initialization
  vTaskDelete(NULL);
}

void setup()
{
  #ifdef DEBUG
  Serial.begin(115200);
  #endif //DEBUG

  xTaskCreatePinnedToCore(TaskInitNetwork,"TaskNetwork",4096,NULL,1,&TaskNetwork_h,ARDUINO_RUNNING_CORE);
   
}

void loop() {

}
