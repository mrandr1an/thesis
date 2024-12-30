#include "wifi.h"
#include "freertos/idf_additions.h"
#include "portal.h"
#include "sdkconfig.h"
#include <esp_wifi_default.h>
#include <esp_wifi.h>
#include <esp_wifi_he_types.h>
#include <esp_err.h>
#include <esp_event.h>
#include <esp_netif.h>
#include <nvs.h>
#include <nvs_flash.h>
#include <string.h>


static void wifi_ap_event(void* arg,esp_event_base_t event_base, int32_t event_id,void* event_data)
{
  ESP_LOGI(AP_TAG, "Handling WiFi Access Point event.");

  switch(event_id)
  {
  case WIFI_EVENT_WIFI_READY: 
    ESP_LOGI(AP_TAG,"WiFi is ready.");
    break;
  case WIFI_EVENT_AP_START:
    ESP_LOGI(AP_TAG,"AP has started.");
    break;
  case WIFI_EVENT_AP_STOP: 
    ESP_LOGI(AP_TAG,"AP has stoped.");
    break;
  case WIFI_EVENT_AP_STACONNECTED:
    ESP_LOGI(AP_TAG,"A station has connected");
    break;
  default:
    ESP_LOGW(AP_TAG, "Event not handled: 0x%." PRIx32, event_id);
  }
}

esp_err_t wifi_init()
{
   ESP_LOGI(APSTA_TAG, "Initializing flash storage.");
   /* Initialize (the default) flash storage partition */
   esp_err_t ret = nvs_flash_init();
   if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        // NVS partition was truncated and needs to be erased
        // Retry nvs_flash_init
        ESP_ERROR_CHECK(nvs_flash_erase());
        ret = nvs_flash_init();
    }

   //Declare flash storage handle
   nvs_handle_t storage_h;
   /* Open the default nvs, its label is "nvs" */
   ret = nvs_open("nvs",NVS_READONLY,&storage_h);

   if (ret != ESP_OK)
   {
     ESP_LOGE(APSTA_TAG, "Could not open flash storage, %s", esp_err_to_name(ret));
     return ret;
   }
 
   ESP_LOGI(APSTA_TAG, "Reading flash storage.");


   char ssid_buffer[32];
   size_t ssid_s;

   /*
     Read "ssid" keys contents and store in ssid_buffer
      - If ESP_OK do the same for the "pwd" key.
         - If ESP_OK connect to WiFi with credentials
	 - Then Setup AP
      - Otherwise contiue to setting up AP
   */
   ret = nvs_get_str(storage_h,"ssid",ssid_buffer,&ssid_s);

   switch (ret) {
            case ESP_OK:
	      ESP_LOGI(APSTA_TAG,"SSID: %s", ssid_buffer);
	      char pwd_buffer[32];
	      size_t pwd_s;
	      ret = nvs_get_str(storage_h,"pwd",pwd_buffer,&pwd_s);
	      //Nested switch statement 
		switch (ret) {
			    case ESP_OK:
			    ESP_LOGI(APSTA_TAG,"PWD: %s", pwd_buffer);
				break;
			    case ESP_ERR_NVS_NOT_FOUND:
			    ESP_LOGI(APSTA_TAG,"Password has not been initialized in NVS: %s.", esp_err_to_name(ret));

			    ret = wifi_ap_init();

			    if (ret != ESP_OK)
			    {
			      ESP_LOGI(APSTA_TAG, "Failed to initialize access point: %s.",esp_err_to_name(ret));
			    }

			    break;
			    default :
			    ESP_LOGI(APSTA_TAG,"Unhandled error on password: %s.", esp_err_to_name(ret));
			}
		 break;
            case ESP_ERR_NVS_NOT_FOUND:
	      ESP_LOGI(APSTA_TAG,"SSID has not been initialized in NVS: %s.", esp_err_to_name(ret));
	      wifi_ap_init();
	      break;
            default :
	      ESP_LOGI(APSTA_TAG,"Unhandled error on ssid %s", esp_err_to_name(ret));
        }
  
  //Close storage
  nvs_close(storage_h);
  return ret;
}

esp_err_t wifi_ap_init()
{
  //Initialize LwIP core task and initialize LwIP-related work.
  esp_err_t ret = esp_netif_init();

  if(ret != ESP_OK)
  {
    ESP_LOGE(AP_TAG, "Could not initialize network interface: %s", esp_err_to_name(ret));
  }

  ret = esp_event_loop_create_default();

  if(ret != ESP_OK)
  {
    ESP_LOGE(AP_TAG, "Could not initialize default loop: %s", esp_err_to_name(ret));
    return ret;
  }

  esp_netif_t* ap_netif =  esp_netif_create_default_wifi_ap();

  assert(ap_netif);

  wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();

  ret = esp_wifi_init(&cfg);

  if(ret != ESP_OK)
  {
    ESP_LOGE(AP_TAG, "Could not initialize default config for wifi access point: %s", esp_err_to_name(ret));
    return ret;
  }

   ESP_ERROR_CHECK(esp_event_handler_register(WIFI_EVENT,ESP_EVENT_ANY_ID,&wifi_ap_event,NULL));

   ESP_ERROR_CHECK( esp_wifi_set_storage(WIFI_STORAGE_RAM) );
   ESP_ERROR_CHECK( esp_wifi_set_mode(WIFI_MODE_NULL) );
   ESP_ERROR_CHECK( esp_wifi_start() );

   wifi_config_t ap_config = {0};

   strcpy((char* ) ap_config.ap.ssid,CONFIG_ESP_WIFI_AP_SSID);
   strcpy((char* ) ap_config.ap.password,CONFIG_ESP_WIFI_AP_PASSWORD);
   ap_config.ap.ssid_len = strlen(CONFIG_ESP_WIFI_AP_SSID);
   ap_config.ap.authmode = WIFI_AUTH_WPA2_PSK;
   ap_config.ap.max_connection = CONFIG_ESP_WIFI_AP_MAX_CONNECTIONS;
   ap_config.ap.channel = CONFIG_ESP_WIFI_AP_CHANNEL; 
   
   ret = esp_wifi_set_mode(WIFI_MODE_APSTA);

   if(ret != ESP_OK)
   {
    ESP_LOGE(AP_TAG, "Could not set WiFi mode to APSTA: %s", esp_err_to_name(ret));
    return ret;
   }

   ret = esp_wifi_set_config(WIFI_IF_AP,&ap_config);
   
   if(ret != ESP_OK)
   {
    ESP_LOGE(AP_TAG, "Could not set WiFi config: %s", esp_err_to_name(ret));
    return ret;
   }

   ret = esp_wifi_start();

   if(ret != ESP_OK)
   {
    ESP_LOGE(AP_TAG, "Could not start access point: %s", esp_err_to_name(ret));
    return ret;
   }

   //Notify HTTP task
   xTaskNotifyGive(hHttpServer);

   return ret;
 }
