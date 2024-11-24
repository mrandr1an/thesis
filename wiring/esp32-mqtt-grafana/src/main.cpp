#define CONFIG
#define DEBUG 
#include <Arduino.h>
#include <WiFi.h>
#include <PubSubClient.h>
#include <HCSR04.h>
#include "Config.hpp"
#include "Topics.hpp"

#ifdef CONFIG
void initWiFi()
{
  #ifdef DEBUG
  Serial.println("Initializing WiFi");
  #endif //DEBUG
  //Set WIFI mode to station
  WiFi.mode(WIFI_STA);
  WiFi.begin(WIFI_NAME,WIFI_PASS);
  #ifdef DEBUG
  Serial.println("Connecting to WiFi");
  #endif //DEBUG
  while (WiFi.status() != WL_CONNECTED)
  { 
  #ifdef DEBUG
  Serial.println("Failed to connect to WiFi");
  #endif //DEBUG
  }

  #ifdef DEBUG
  Serial.print("Connect to WiFi network:");
  //This pointer should always be valid after the while loop
  Serial.println(WiFi.SSID());

  Serial.print("IP:");
  Serial.println(WiFi.localIP());
  #endif //DEBUG
}

WiFiClient wifiClient;
PubSubClient mqttClient(wifiClient);

void initMQTT()
{
  #ifdef DEBUG
  Serial.println("Initializing MQTT Client");
  #endif //DEBUG
  mqttClient.setServer(MQTT_BROKER_IP,MQTT_BROKER_PORT);

  while (!mqttClient.connected())
  {
    #ifdef DEBUG
    Serial.println("Failed to connect to MQTT Broker");
    #endif //DEBUG
    mqttClient.connect("esp32");
  }
  mqttClient.loop();
  #ifdef DEBUG
  Serial.println("Connected to MQTT Broker");
  #endif //DEBUG
}

#endif //CONFIG

#define trigPin 5
#define echoPin 18
void initSensors()
{
  HCSR04.begin(trigPin,echoPin);
}

void setup()
{
#ifdef DEBUG
  Serial.begin(115200);
#endif //DEBUG
#ifdef CONFIG
  initWiFi();
  initMQTT();
#else
#ifdef DEBUG
  Serial.println("Configuration not added, compile with CONFIG flag if needed. Continuing") 
#endif //DEBUG
#endif //CONFIG
   initSensors();
}

void loop()
{
  double* distances = HCSR04.measureDistanceCm();
  TopicHCSR04 topic(distances[0]);
  Serial.print(topic.intoJSON()); 
  mqttClient.publish("esp32/hcsr04", topic.intoJSON().c_str());
}

