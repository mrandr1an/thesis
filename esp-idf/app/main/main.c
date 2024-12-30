#include "wifi.h"
#include "portal.h"

#include <stdio.h>
#include <esp_spiffs.h>
#include <esp_err.h>
#include <freertos/idf_additions.h>

#define MAIN_TAG "MAIN"

void app_main(void)
{

  printf("Hello world!\n");
  /* INIT SOFTWARE */
  //Task Initializing
  xTaskCreate(vHttpServer,"HTTP_SERVER",4096,NULL,1,&hHttpServer);

  //Init Wifi
  esp_err_t main_ret = wifi_init();

  if(main_ret != ESP_OK)
  {
    ESP_LOGE(MAIN_TAG,"Could not initialize WiFi: %s", esp_err_to_name(main_ret));
  }

  //Init Storage
  esp_vfs_spiffs_conf_t config = {
    .base_path = "/storage",
    .partition_label = NULL,
    .max_files = 5,
    .format_if_mount_failed = true,
  };
 
  main_ret = esp_vfs_spiffs_register(&config);

  if(main_ret != ESP_OK)
  {
    ESP_LOGE(MAIN_TAG,"Could not initialize SPIFF storage: %s", esp_err_to_name(main_ret));
  }
}
