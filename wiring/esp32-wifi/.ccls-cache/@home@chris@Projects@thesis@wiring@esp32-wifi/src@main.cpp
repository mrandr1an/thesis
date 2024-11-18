#include <Arduino.h>
#include <WiFi.h>
#include "Config/Config.hpp"
#include <ThingsBoard.h>

#define TOKEN "JClISUGAid273i6ts7cX"
#define TB_SERVER "192.168.2.2"

WiFiClient espClient;
ThingsBoard tb(espClient);

void initWiFi() {
  WiFi.mode(WIFI_STA);
  WiFi.begin(WIFI_NAME,WIFI_PASS);
  Serial.println("Connecting to WiFi...");
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print('.');
    Serial.println(WiFi.status());
    delay(1000);
  }

  while (!tb.connected()) {
    Serial.println("Not Connected to Thingsboard");
    if(!tb.connect(TB_SERVER, TOKEN)){
       Serial.println("Still not connected");
       delay(500);
    } else{
    Serial.println("Connected to Thingsboard");;
    break;
    }
  }
  pinMode(5,OUTPUT);
}

void setup() {
  // Set pin mode
  Serial.begin(115200);
  Serial.println("Setting ESP32 as Station");
  initWiFi();
}

int illuminance = 0;
void loop()
{
  //Fake data
  Serial.println("Printing fake data");
  tb.sendTelemetryData("illuminance",illuminance);
  illuminance += 10;
  delay(1000);
}
