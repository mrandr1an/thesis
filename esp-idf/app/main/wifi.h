#pragma once
#include "esp_netif_types.h"
#include "freertos/idf_additions.h"
#include <esp_event_base.h>
#include <esp_err.h>
#include <esp_log.h>

/* TAG Definitions used for pretty log printing */
#define STA_TAG "WiFi_STA"
#define AP_TAG "WiFi_AP"
#define APSTA_TAG "WiFi"

/*State Bits for FreeRTOS Event Groups*/
#define STA_CONNECTED_BIT BIT0
#define STA_CONNECTION_FAILED_BIT BIT1

/* Event Groups */
static EventGroupHandle_t wifi_event_group = NULL;
static esp_netif_t *netif = NULL;
static esp_event_handler_instance_t ip_event_handler;
static esp_event_handler_instance_t wifi_sta_event_handler;
static esp_event_handler_instance_t wifi_ap_event_handler;

/* Global State Variables */
static int wifi_retry_count = 0;

// Init Functions

static esp_err_t wifi_sta_init(void);
static esp_err_t wifi_ap_init(void);
esp_err_t wifi_init(void);

// Event Handlers
static void ip_event(void* arg,esp_event_base_t event_base, int32_t event_id,void* event_data);
static void wifi_sta_event(void* arg,esp_event_base_t event_base, int32_t event_id,void* event_data);
static void wifi_ap_event(void* arg,esp_event_base_t event_base, int32_t event_id,void* event_data);

// Action Functions
static esp_err_t wifi_sta_connect(char* ssid, char* pwd);
