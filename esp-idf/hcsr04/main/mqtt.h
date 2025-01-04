#pragma once

#include <freertos/idf_additions.h>
#include <esp_err.h>
#include <esp_event_base.h>
#include <mqtt_client.h>

#define MQTT_TAG "MQTT"

extern TaskHandle_t mqttPublisher_h;
extern TaskHandle_t mqttClient_h;

extern esp_mqtt_client_handle_t mqttClient;

static void mqtt_event_handler(void *handler_args, esp_event_base_t base,
                               int32_t event_id, void *event_data);

static esp_err_t mqtt_start(void);

void MQTT_Task(void* pvParameters);
void MQTT_Publisher_Task(void* pvParameters);
