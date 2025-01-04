#include "mqtt.h"
#include "freertos/idf_additions.h"
#include "hcsr04.h"
#include "sdkconfig.h"
#include <mqtt_client.h>
#include <esp_log.h>

TaskHandle_t mqttClient_h = NULL;
TaskHandle_t mqttPublisher_h = NULL;

static void mqtt_event_handler(void *handler_args, esp_event_base_t base,
                               int32_t event_id, void *event_data)
{
  ESP_LOGI(MQTT_TAG, "Handling MQTT event.");
  esp_mqtt_event_handle_t event = event_data;
  esp_mqtt_client_handle_t client = event->client;
  int msg_id;
  switch((esp_mqtt_event_id_t) event_id)
  {
    case MQTT_EVENT_CONNECTED:
      ESP_LOGI(MQTT_TAG,"MQTT Connected.");
      break;
    case MQTT_EVENT_DISCONNECTED:
      ESP_LOGI(MQTT_TAG,"MQTT Disconnected.");
      break;
    case MQTT_EVENT_SUBSCRIBED:
      ESP_LOGI(MQTT_TAG,"MQTT Subscribed.");
      break;
    case MQTT_EVENT_UNSUBSCRIBED:
      ESP_LOGI(MQTT_TAG,"MQTT Unsubscribed.");
      break;
    case MQTT_EVENT_PUBLISHED:
      ESP_LOGI(MQTT_TAG,"MQTT Published.");
      break;
    case MQTT_EVENT_BEFORE_CONNECT:
      ESP_LOGI(MQTT_TAG,"MQTT before connect event.");
      break;
    case MQTT_EVENT_ERROR:
      ESP_LOGE(MQTT_TAG,"MQTT Error: %d.", event->error_handle->error_type);
     break;
    case MQTT_EVENT_DATA:
      ESP_LOGI(MQTT_TAG,"MQTT Data.");
     break;
    default:
      ESP_LOGI(MQTT_TAG, "Unhandled event: %0x", event->event_id);
      break;
  }
}

esp_mqtt_client_handle_t mqttClient = NULL;

esp_err_t mqtt_start(void)
{

  ESP_LOGI(MQTT_TAG, "Starting MQTT Client");

  esp_mqtt_client_config_t mqttConfig = {
    .broker.address.uri = CONFIG_MQTT_BROKER_URI,
    .credentials.client_id = CONFIG_MQTT_CLIENT_ID,
    .credentials.username = CONFIG_MQTT_CLIENT_USERNAME,
    .credentials.authentication.password = CONFIG_MQTT_CLIENT_PASSWORD,
    .session.protocol_ver = MQTT_PROTOCOL_V_5,
    
  };

  mqttClient = esp_mqtt_client_init(&mqttConfig);

  esp_mqtt_client_register_event(mqttClient, ESP_EVENT_ANY_ID,mqtt_event_handler,mqttClient);
  return esp_mqtt_client_start(mqttClient);
}

void MQTT_Task(void* pvParameters)
{
  ulTaskNotifyTake(pdTRUE,portMAX_DELAY);
  esp_err_t ret =  mqtt_start();
  if(ret != ESP_OK)
  {
    ESP_LOGE(MQTT_TAG, "Could not start MQTT Service: %s.",esp_err_to_name(ret));
  }

  xTaskNotifyGive(mqttPublisher_h); 

  vTaskDelete(NULL);
}

void MQTT_Publisher_Task(void* pvParameters)
{
  ulTaskNotifyTake(pdTRUE,portMAX_DELAY);
  float distance;

  while(1)
 {
    if (xQueueReceive(distance_queue, &distance, portMAX_DELAY) == pdPASS)
    {
      char message[32];
      snprintf(message, sizeof(message), "{\"distance\":%.2f}", distance * 100);
      esp_mqtt_client_publish(mqttClient, "sensor/distance", message, 0, 1, 0);
      ESP_LOGI(MQTT_TAG, "Published distance: %.2f cm", distance * 100);
    }
  }
}
