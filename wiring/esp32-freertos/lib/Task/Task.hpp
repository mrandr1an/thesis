#ifndef TASK_H
#define TASK_H

#include <WiFi.h>
#include <PubSubClient.h>
#include <HCSR04.h>
#include <esp_log.h>
#include <ArduinoJson.h>
#include "Config.hpp"

/* Task Handles */
TaskHandle_t TaskNetwork_h;
TaskHandle_t TaskHW_h;
TaskHandle_t TaskHCSR04_h;
TaskHandle_t TaskPublish_h;

struct NetworkState
{
public:
  String broker_ip;
  String clientId;
  String local_ip;
  String networkName;
  uint16_t broker_port;
  int8_t RSSI; 
};

struct DeviceData
{
  NetworkState* nwState;
  double distance;
};

void TaskInitNetwork(void* pvParameters);

#define HCSR04_TRIG_PIN 18
#define HCSR04_ECHO_PIN 5

void TaskInitHW(void* pvParameters);

void TaskSenseDistance(void* pvParameters);

void TaskPublish(void* pvParameters);

#endif //TASK_H
