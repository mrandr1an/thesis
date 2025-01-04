#include "wifi.h"
#include "esp_netif_types.h"
#include "mqtt.h"
#include "webserver.h"

#include <esp_err.h>
#include <esp_event_base.h>
#include <esp_system.h>
#include <esp_wifi_types_generic.h>
#include <esp_log.h>
#include <esp_wifi_default.h>
#include <esp_wifi.h>
#include <esp_wifi_he_types.h>
#include <nvs.h>
#include <nvs_flash.h>
#include <string.h>

TaskHandle_t WiFi_h = NULL;

static void wifi_event(void* arg,esp_event_base_t event_base, int32_t event_id,void* event_data)
{
  ESP_LOGI(WIFI_TAG, "Handling WiFi event.");
  switch(event_id)
  {
   case WIFI_EVENT_WIFI_READY:
     ESP_LOGI(WIFI_TAG,"WiFi is ready."); 
     break;
   case WIFI_EVENT_STA_START:
     ESP_LOGI(WIFI_TAG,"WiFi STA started.");
     break;
   case WIFI_EVENT_STA_CONNECTED:
     ESP_LOGI(WIFI_TAG,"WiFi STA connected to a Network.");
     xTaskNotifyGive(mqttClient_h); 
     break;
   case WIFI_EVENT_AP_START:
     ESP_LOGI(WIFI_TAG,"WiFi AP started."); 
     break;
   case WIFI_EVENT_AP_STACONNECTED:
     ESP_LOGI(WIFI_TAG,"WiFi station has connected to AP."); 
     break;
   case WIFI_EVENT_AP_STADISCONNECTED:
     ESP_LOGI(WIFI_TAG,"WiFi station has disconnected from AP."); 
     break;
   case WIFI_EVENT_STA_BSS_RSSI_LOW: 
     ESP_LOGW(WIFI_TAG,"WiFi RSSI is below the configured threshold."); 
     break;
   case WIFI_EVENT_HOME_CHANNEL_CHANGE:  
     ESP_LOGW(WIFI_TAG,"WiFi home channel has changed."); 
     break;
  default:
     ESP_LOGW(WIFI_TAG, "WiFi Event not handled: 0x%." PRIx32, event_id);
  }
}

static void ip_event(void *arg, esp_event_base_t event_base, int32_t event_id, void *event_data) {
  ESP_LOGI(WIFI_TAG, "Handling IP event.");
  switch(event_id) {
    case IP_EVENT_STA_GOT_IP:
      ip_event_got_ip_t* event = (ip_event_got_ip_t *)event_data;
      ESP_LOGI(WIFI_TAG, "Got IP address: " IPSTR, IP2STR(&event->ip_info.ip));
      break;
    default:
      ESP_LOGW(WIFI_TAG, "IP Event not handled: 0x%" PRIx32, event_id);
   }
}


static esp_err_t wifi_init()
{
   ESP_LOGI(WIFI_TAG, "Initializing WiFi.");
   esp_err_t ret = nvs_flash_init();

   if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
    // NVS partition was truncated and needs to be erased
    // Retry nvs_flash_init
    ESP_ERROR_CHECK(nvs_flash_erase());
    ret = nvs_flash_init();
  }

  ret = esp_netif_init();

  if (ret != ESP_OK)
  {
    ESP_LOGE(WIFI_TAG,"Failed to initialize network interface: %s.", esp_err_to_name(ret));
    return ret;
  }

  wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();

  ret = esp_wifi_init(&cfg);

  if(ret != ESP_OK)
  {
    ESP_LOGE(WIFI_TAG,"Failed to initialize default config for wifi: %s.", esp_err_to_name(ret));
    return ret;
  }

   if(ret != ESP_OK)
   {
    ESP_LOGE(WIFI_TAG,"Failed to register event handler for wifi: %s.", esp_err_to_name(ret));
    return ret;
   }

   ret = esp_event_loop_create_default();

   if(ret != ESP_OK)
   {
    ESP_LOGE(WIFI_TAG, "Could not initialize default loop: %s", esp_err_to_name(ret));
    return ret;
   }

   ret = esp_event_handler_register(WIFI_EVENT,ESP_EVENT_ANY_ID,&wifi_event,NULL);
   if(ret != ESP_OK)
   {
    ESP_LOGE(WIFI_TAG, "Could not register WIFI_EVENT handler to the default event loop: %s.", esp_err_to_name(ret));
    return ret;
   }

   ret = esp_event_handler_register(IP_EVENT,ESP_EVENT_ANY_ID,&ip_event,NULL);
   if(ret != ESP_OK)
   {
    ESP_LOGE(WIFI_TAG, "Could not register IP_EVENT handler to the default event loop: %s.", esp_err_to_name(ret));
    return ret;
   }

   ESP_ERROR_CHECK( esp_wifi_set_storage(WIFI_STORAGE_RAM) );
   ESP_ERROR_CHECK( esp_wifi_set_mode(WIFI_MODE_APSTA) );

   /* AP Config Setup */

   esp_netif_t* ap_netif =  esp_netif_create_default_wifi_ap();
   assert(ap_netif);

   wifi_config_t ap_config = {0};

   strcpy((char* ) ap_config.ap.ssid,CONFIG_ESP_WIFI_AP_SSID);
   strcpy((char* ) ap_config.ap.password,CONFIG_ESP_WIFI_AP_PASSWORD);
   ap_config.ap.ssid_len = strlen(CONFIG_ESP_WIFI_AP_SSID);
   ap_config.ap.authmode = WIFI_AUTH_WPA2_PSK;
   ap_config.ap.max_connection = CONFIG_ESP_WIFI_AP_MAX_CONNECTIONS;
   ap_config.ap.channel = CONFIG_ESP_WIFI_AP_CHANNEL; 
   ap_config.ap.beacon_interval = 100;  // Default is 100 ms

   ret = esp_wifi_set_config(WIFI_IF_AP,&ap_config);

   esp_netif_t* sta_netif =  esp_netif_create_default_wifi_sta();
   assert(sta_netif);

   if(ret != ESP_OK)
   {
     ESP_LOGE(WIFI_TAG,"Could not set Access Point config: %s.", esp_err_to_name(ret));
     return ret;
   }
   
   /* Start WiFi with current configuration */
   ret = esp_wifi_start();

   if(ret != ESP_OK)
   {
    ESP_LOGE(WIFI_TAG,"Failed to start wifi: %s.", esp_err_to_name(ret));
    return ret;
   }

   //Notify HTTP task 
   xTaskNotifyGive(HttpServer_h);

   //Declare flash storage handle
   nvs_handle_t storage_h;
   /* Open the default nvs, its label is "nvs" */
   ret = nvs_open("nvs",NVS_READONLY,&storage_h);

   if (ret != ESP_OK)
   {
     ESP_LOGE(WIFI_TAG, "Could not open flash storage, %s", esp_err_to_name(ret));
     return ret;
   }

   char ssid_buffer[32], pwd_buffer[32];
   size_t ssid_s,pwd_s;
   esp_err_t got_ssid,got_pwd;

   got_ssid = nvs_get_str(storage_h,"ssid",ssid_buffer,&ssid_s);
   got_pwd = nvs_get_str(storage_h,"pwd",pwd_buffer,&pwd_s);

   if(got_ssid == ESP_OK && got_pwd == ESP_OK)
   {
     ESP_LOGI(WIFI_TAG, "Using nvs credentials to log into network %s.", ssid_buffer);
     wifi_connect(ssid_buffer,pwd_buffer);
   }
   
  return ret;
}

static esp_err_t wifi_deinit()
{ 
  ESP_LOGI(WIFI_TAG,"Uninitializing WiFi.");
  return esp_wifi_deinit();
}

void WiFi_Task(void* pvParameters)
{
  esp_err_t ret = wifi_init();

  if(ret != ESP_OK)
  {
    ESP_LOGE(WIFI_TAG,"Could not initialize WiFi: %s.",esp_err_to_name(ret));
  }

  vTaskDelete(NULL);
}

esp_err_t wifi_connect(char* ssid, char* pwd)
{

   /* STA Config Setup */
   wifi_config_t sta_config = {0};
   strcpy((char*)sta_config.sta.ssid,ssid); 
   strcpy((char*)sta_config.sta.password, pwd);
   sta_config.sta.threshold.authmode = WIFI_AUTH_WPA2_PSK;
   sta_config.sta.pmf_cfg.capable = true;
   sta_config.sta.pmf_cfg.required = false;
  
   esp_err_t ret = esp_wifi_set_config(WIFI_IF_STA, &sta_config);

   if (ret != ESP_OK) {
    ESP_LOGE(WIFI_TAG, "Could not set Station config: %s.", esp_err_to_name(ret));
    return ret;
   }

   ret = esp_wifi_disconnect();

   if (ret != ESP_OK) {
    ESP_LOGE(WIFI_TAG, "Could not disconnect before connecting.: %s.", esp_err_to_name(ret));
    return ret;
   }

   ret = esp_wifi_connect();
   
   if (ret != ESP_OK) {
    ESP_LOGE(WIFI_TAG, "Could not connect to network: %s.", esp_err_to_name(ret));
    return ret;
   }

   return ret;
}
