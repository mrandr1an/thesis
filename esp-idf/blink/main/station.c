
#include "station.h"
#include "esp_wifi_default.h"
#include "nvs_flash.h"

const int CONNECTED_BIT = BIT0;
const int FAIL_BIT = BIT1;

static void ip_handler(void *arg, esp_event_base_t event_base, int32_t event_id,
                       void *event_data)
{

  ESP_LOGI(STA_TAG, "Handling IP events 0x%" PRIx32, event_id);
  switch(event_id)
  {
  case(IP_EVENT_STA_GOT_IP):
    ip_event_got_ip_t *event_ip = (ip_event_got_ip_t*) event_data;
    ESP_LOGI(STA_TAG,"Got IP: " IPSTR,IP2STR(&event_ip->ip_info.ip));
    xEventGroupSetBits(wifi_event_group, CONNECTED_BIT);
    break;
    case (IP_EVENT_STA_LOST_IP):
        ESP_LOGI(STA_TAG, "Lost IP");
   default:
        ESP_LOGI(STA_TAG, "IP event not handled");
        break;
  }
}

static void wifi_sta_handler(void* arg, esp_event_base_t event_base,
								int32_t event_id, void* event_data)
{
  ESP_LOGI(STA_TAG, "Handling WiFi Station 0x%" PRIx32, event_id);
  switch(event_id)
  {
  case(WIFI_EVENT_WIFI_READY):
    ESP_LOGI(STA_TAG, "WiFi ready");
    break;
  case(WIFI_EVENT_SCAN_DONE):
    ESP_LOGI(STA_TAG, "WiFi Access Point scan done");
    break;
  case(WIFI_EVENT_STA_START):
    ESP_LOGI(STA_TAG, "WiFi Station start");
    esp_wifi_connect();
    break;
  case(WIFI_EVENT_STA_STOP):
    ESP_LOGI(STA_TAG, "WiFi Station stop");
    break;
  case(WIFI_EVENT_STA_DISCONNECTED):
    ESP_LOGI(STA_TAG, "WiFi Station disconnected from AP");
     if (wifi_retry_count < WIFI_RETRY_ATTEMPT) {
            ESP_LOGI(STA_TAG, "Retrying to connect to Wi-Fi network...");
            esp_wifi_connect();
            wifi_retry_count++;
        } else {
            ESP_LOGI(STA_TAG, "Failed to connect to Wi-Fi network");
            xEventGroupSetBits(wifi_event_group, FAIL_BIT);
        }
    break;
  case(WIFI_EVENT_STA_CONNECTED):
    ESP_LOGI(STA_TAG, "WiFi Station connected to AP");
    break;
  default:
        ESP_LOGI(STA_TAG, "Wi-Fi event not handled");
        break;
  }
}

void init_wifi_apsta()
{
  esp_log_level_set(AP_TAG,ESP_LOG_WARN);
  esp_log_level_set(STA_TAG,ESP_LOG_WARN);
    esp_err_t ret = nvs_flash_init();
	if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
	    ESP_ERROR_CHECK(nvs_flash_erase());
	    ret = nvs_flash_init();
	}

	static bool initialized = false;
	if (initialized) {
		return;
	}
	ESP_ERROR_CHECK(esp_netif_init());
	wifi_event_group = xEventGroupCreate();
	ESP_ERROR_CHECK(esp_event_loop_create_default());

	esp_wifi_set_default_wifi_sta_handlers();
	esp_netif_create_default_wifi_sta();

	wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
	ESP_ERROR_CHECK( esp_wifi_init(&cfg) );
	ESP_ERROR_CHECK( esp_event_handler_register(WIFI_EVENT, ESP_EVENT_ANY_ID, &wifi_sta_handler, NULL) );
	ESP_ERROR_CHECK( esp_event_handler_register(IP_EVENT, ESP_EVENT_ANY_ID, &ip_handler, NULL) );
	
	ESP_ERROR_CHECK( esp_wifi_set_storage(WIFI_STORAGE_RAM) );
	ESP_ERROR_CHECK( esp_wifi_set_mode(WIFI_MODE_NULL) );
	ESP_ERROR_CHECK( esp_wifi_start() );
}


esp_err_t wifi_sta_connect(char *wifi_ssid, char *wifi_pwd)
{
   wifi_config_t wifi_config = {
        .sta = {
            // this sets the weakest authmode accepted in fast scan mode (default)
            .threshold.authmode = WIFI_AUTHMODE,
        },
    };

    strncpy((char*)wifi_config.sta.ssid, wifi_ssid, sizeof(wifi_config.sta.ssid));
    strncpy((char*)wifi_config.sta.password, wifi_pwd, sizeof(wifi_config.sta.password));

    ESP_ERROR_CHECK(esp_wifi_set_ps(WIFI_PS_NONE)); // default is WIFI_PS_MIN_MODEM
    ESP_ERROR_CHECK(esp_wifi_set_storage(WIFI_STORAGE_RAM)); // default is WIFI_STORAGE_FLASH

    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));
    ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_STA, &wifi_config));

    ESP_LOGI(STA_TAG, "Connecting to Wi-Fi network: %s", wifi_config.sta.ssid);
    ESP_ERROR_CHECK(esp_wifi_start());

    EventBits_t bits = xEventGroupWaitBits(wifi_event_group, CONNECTED_BIT | FAIL_BIT,
        pdFALSE, pdFALSE, portMAX_DELAY);

    if (bits & CONNECTED_BIT) {
        ESP_LOGI(STA_TAG, "Connected to Wi-Fi network: %s", wifi_config.sta.ssid);
        return ESP_OK;
    } else if (bits & FAIL_BIT) {
        ESP_LOGE(STA_TAG, "Failed to connect to Wi-Fi network: %s", wifi_config.sta.ssid);
        return ESP_FAIL;
    }

    ESP_LOGE(STA_TAG, "Unexpected Wi-Fi error");
    return ESP_FAIL;

}
