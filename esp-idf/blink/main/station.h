#pragma once

#include <freertos/idf_additions.h>
#include <esp_err.h>
#include <esp_event_base.h>
#include <esp_netif_ip_addr.h>
#include <esp_netif_types.h>
#include <esp_wifi_types_generic.h>
#include <esp_log.h>
#include <stdbool.h>
#include <esp_wifi.h>
#include <freertos/event_groups.h>
#include "esp_wifi_types_generic.h"
#include <string.h>

#define WIFI_AUTHMODE WIFI_AUTH_WPA2_PSK
static const int WIFI_RETRY_ATTEMPT = 3;

static int wifi_retry_count = 0;

static EventGroupHandle_t wifi_event_group;

static const char* AP_TAG = "WiFi AP";
static const char* STA_TAG = "WiFi STA";


/* Event Handlers */
static void ip_handler(void *arg, esp_event_base_t event_base, int32_t event_id,
                       void *event_data);

static void wifi_sta_handler(void* arg, esp_event_base_t event_base,
			     int32_t event_id, void* event_data);

/* Initialization Functions */

void init_wifi_apsta(void);


/* Action Functions */

esp_err_t wifi_sta_connect(char* wifi_ssid,char* wifi_pwd);
