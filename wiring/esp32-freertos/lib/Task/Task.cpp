#include "Task.hpp"

WiFiClient wifiClient;
PubSubClient mqttClient(wifiClient);

NetworkState Network_s;
DeviceData Data = DeviceData
{
  &Network_s,
  -1,
};

void TaskInitNetwork(void* pvParameters)
{

  ESP_LOGI("NETWORK","Network is initializing");
  //BEGIN INIT WIFI

  //Timeout timer
  WiFi.mode(WIFI_STA);
  WiFi.begin(WIFI_NAME,WIFI_PASS);
  
  int timeout = 0;
  ESP_LOGI("NETWORK", "Connecting to Wi-Fi...");
  while (timeout < WIFI_TIMEOUT_SECONDS)
  {
    if (WiFi.status() != WL_CONNECTED)
    {
      ESP_LOGI("NETWORK", "Waiting for Wi-Fi...");
      vTaskDelay(pdMS_TO_TICKS(1000));
      timeout++;
    } else {
      break;
    }
  }
  ESP_LOGI("NETWORK", "Wi-Fi connection status: %d", WiFi.status());
  //END INIT WIFI

  //BEGIN INIT MQTT
  if(WiFi.status() == WL_CONNECTED) {
    ESP_LOGI("NETWORK", "Wi-Fi connected, RSSI: %d", WiFi.RSSI());
    mqttClient.setServer(MQTT_BROKER_IP,MQTT_BROKER_PORT);
    //Reset timeout timer
    timeout = 0;
    mqttClient.connect("esp32");
    ESP_LOGI("NETWORK", "Connecting to MQTT broker...");
    while (timeout < MQTT_TIMEOUT_SECONDS)
    {
	if (mqttClient.state() != MQTT_CONNECTED) {
	vTaskDelay(pdMS_TO_TICKS(1000));
	timeout++;
        } else {
	  break;
        }
    }
    ESP_LOGI("NETWORK", "MQTT connection state: %d", mqttClient.state());
    if (mqttClient.state() == MQTT_CONNECTED)
    {
      Network_s.RSSI = WiFi.RSSI();
      Network_s.broker_ip = MQTT_BROKER_IP;
      Network_s.broker_port = MQTT_BROKER_PORT;
      Network_s.local_ip = WiFi.localIP().toString();
      Network_s.networkName = WiFi.BSSIDstr();
      Network_s.clientId = "esp32";
    } else {
      Network_s.RSSI = WiFi.RSSI();
      Network_s.broker_ip = "None";
      Network_s.broker_port = 0;
      Network_s.local_ip = WiFi.localIP().toString();
      Network_s.networkName = WiFi.BSSIDstr();
      Network_s.clientId = "None";
    }
  } else
  {
    Network_s.RSSI = 0;
    Network_s.broker_ip = "None";
    Network_s.broker_port = 0;
    Network_s.local_ip = "None";
    Network_s.networkName = "None";
    Network_s.clientId = "None";
  }
  //END INIT MQTT
  ESP_LOGI("NETWORK","Network is done initializing");
  
  //Delete task after init
  vTaskDelete(NULL);
}

void TaskInitHW(void *pvParameters)
{
  ESP_LOGI("HARDWARE","Hardware is initializing");

  HCSR04.begin(HCSR04_TRIG_PIN,HCSR04_ECHO_PIN);

  ESP_LOGI("HARDWARE","Hardware is done initializing");

  xTaskNotifyGive(TaskHCSR04_h);

  //Delete task after init
  vTaskDelete(NULL);
}

void TaskSenseDistance(void *pvParameters)
{
   static uint32_t thread_notification = ulTaskNotifyTake(pdTRUE,portMAX_DELAY);
    while(thread_notification)
    {
        double* distances = HCSR04.measureDistanceCm();
	ESP_LOGI("HC-SR04","Distance is: %f",distances[0]);
	Data.distance = distances[0];

	xTaskNotifyGive(TaskPublish_h);
        vTaskDelay(pdMS_TO_TICKS(500));
    }
}

void TaskPublish(void *pvParameters)
{
    char buffer[256]; // Allocate the buffer once
    for (;;)
    {
        // Wait for a notification indefinitely
        uint32_t thread_notification = ulTaskNotifyTake(pdTRUE, portMAX_DELAY);
        
        // Check if got notification
        if (thread_notification)
        {
            // Create and populate JSON document
            JsonDocument doc;
            doc["distance"] = Data.distance;
            doc["rssi"] = Data.nwState->RSSI;
            doc["local_ip"] = Data.nwState->local_ip;
            doc["network_name"] = Data.nwState->networkName;

            // Serialize and publish JSON
            serializeJson(doc, buffer);
            mqttClient.publish("esp32/data", buffer);
            ESP_LOGI("MQTT", "Data published");
        }
    }
}
