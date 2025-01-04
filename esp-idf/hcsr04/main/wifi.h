#pragma once

#include <esp_event_base.h>
#include <esp_err.h>
#include <freertos/idf_additions.h>

#define WIFI_TAG "WiFi"

extern TaskHandle_t WiFi_h;

/* General WiFi Events */
static void wifi_event(void* arg,esp_event_base_t event_base, int32_t event_id,void* event_data);
static void ip_event(void *arg, esp_event_base_t event_base, int32_t event_id, void *event_data);

/* Initializes the WiFi stack and the NVS stack */
static esp_err_t wifi_init();

/* Deinitializes the WiFi stack */
static esp_err_t wifi_deinit();

/* Connect as Station to a Network */
esp_err_t wifi_connect(char* ssid, char* pwd);

/* Task that calls wifi_init() */
void WiFi_Task(void* pvParameters);
