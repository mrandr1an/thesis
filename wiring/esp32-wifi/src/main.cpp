#include <Arduino.h>
#include <WiFi.h>
#include "Config/Config.hpp"

#define LED 2

void initWiFi() {
  WiFi.mode(WIFI_STA);
  WiFi.begin(WIFI_NAME,WIFI_PASS);
  Serial.println("Connecting to WiFi...");
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print('.');
    Serial.println(WiFi.status());
    delay(1000);
  }
}

void setup() {
  // Set pin mode
  Serial.begin(115200);
  Serial.println("Setting ESP32 as Station");
  initWiFi();
  pinMode(LED,OUTPUT);
}

void loop()
{
  Serial.println(WiFi.localIP());
  Serial.println(WiFi.RSSI());
  delay(1000);
}
