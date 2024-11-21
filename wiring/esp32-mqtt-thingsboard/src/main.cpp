
#include <Arduino.h>
#include "Config.hpp"
#include <WiFi.h>
#include <PubSubClient.h>
#include <ArduinoJson.h>

WiFiClient wifiClient;
PubSubClient mqttClient(wifiClient);
#define DEBUG

void initWiFi()
{
 #ifdef DEBUG
  Serial.println("Initializing WiFi...");
 #endif //DEBUG
  
 WiFi.begin(WIFI_NAME,WIFI_PASS);
 while (WiFi.status() != WL_CONNECTED)
 {
    #ifdef DEBUG
      Serial.println("Connecting...");
    #endif //DEBUG
 }
    #ifdef DEBUG
      Serial.println("Connected");
      Serial.print("Local Ip Address:");
      Serial.println(WiFi.localIP());
    #endif //DEBUG
}

void initMQTT()
{
 #ifdef DEBUG
  Serial.println("Initializing MQTT...");
 #endif //DEBUG 

 mqttClient.setServer(MQTT_BROKER,MQTT_BROKER_PORT);
 String client_id = "esp32-client-";
 client_id += String(WiFi.macAddress());
 while (!mqttClient.connected())
 {
   #ifdef DEBUG
    Serial.print(client_id);
    Serial.println(" attemping to connect to MQQT broker");
   #endif //DEBUG
   boolean connected = mqttClient.connect(client_id.c_str());
   #ifdef DEBUG
   if (connected) {
    Serial.println("Client connected to broker");
   } else {
    Serial.println("Client failed to connect to broker");
    Serial.print("Client State: ");
    Serial.println(mqttClient.state());
   }
   #endif //DEBUG
 }
}

void setup()
{
  #ifdef DEBUG
   Serial.begin(115200);
  #endif //DEBUG
  initWiFi();
  initMQTT();
}

void loop()
{
  char buffer[256];
  JsonDocument doc;
  doc["temp"] = 73;
  doc["profile"] = "something";
  serializeJson(doc, buffer);

  mqttClient.publish("esp32/temperature", buffer);
  #ifdef DEBUG
   Serial.println("Published topic");
   Serial.println(buffer);
   delay(1000);
  #endif //DEBUG
}
