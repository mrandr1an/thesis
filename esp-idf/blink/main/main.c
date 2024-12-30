#include <stdio.h>

#include "esp_log.h"
#include "esp_wifi.h"

#include "freertos/task.h"

#include "station.h"

#define TAG "MAIN"

void app_main(void) {
  init_wifi_apsta();
  esp_err_t ret = wifi_sta_connect("Andrian","12345678");

  wifi_ap_record_t ap_info;
    ret = esp_wifi_sta_get_ap_info(&ap_info);
    if (ret == ESP_ERR_WIFI_CONN) {
        ESP_LOGE(TAG, "Wi-Fi station interface not initialized");
    }
    else if (ret == ESP_ERR_WIFI_NOT_CONNECT) {
        ESP_LOGE(TAG, "Wi-Fi station is not connected");
    } else {
        ESP_LOGI(TAG, "--- Access Point Information ---");
        ESP_LOG_BUFFER_HEX("MAC Address", ap_info.bssid, sizeof(ap_info.bssid));
        ESP_LOG_BUFFER_CHAR("SSID", ap_info.ssid, sizeof(ap_info.ssid));
        ESP_LOGI(TAG, "Primary Channel: %d", ap_info.primary);
        ESP_LOGI(TAG, "RSSI: %d", ap_info.rssi);

        ESP_LOGI(TAG, "Disconnecting in 5 seconds...");
        vTaskDelay(pdMS_TO_TICKS(5000));
    }

}
