#include "esp_log.h"
#include "freertos/idf_additions.h"
#include "hcsr04.h"
#include "mqtt.h"
#include "webserver.h"
#include "wifi.h"
#include <esp_spiffs.h>
#include <sdkconfig.h>
#include <stdio.h>

void app_main()
{ 
  //Init Hardware
  xTaskCreate(HCSR04_init_Task,"HCSR04_INIT",4024,NULL,2,&HCSR04_Init_h);

  //Init Software
  esp_vfs_spiffs_conf_t config = {
    .base_path = "/storage",
    .partition_label = NULL,
    .max_files = 5,
    .format_if_mount_failed = true,
  };

  esp_err_t ret = esp_vfs_spiffs_register(&config);

  if(ret != ESP_OK)
  {
    ESP_LOGE("MAIN_TAG", "Could not init spiffs: %s.", esp_err_to_name(ret));
  }

  xTaskCreate(WiFi_Task,"WiFi",4024,NULL,2,&WiFi_h);
  xTaskCreate(HttpServer_Task,"WebServer",4024,NULL,1,&HttpServer_h);
  xTaskCreate(MQTT_Task, "MQTT",4024,NULL,1,&mqttClient_h);

  //Running
  xTaskCreate(HCSR04_measure_Task,"HCSR04",4024,NULL,0,&HCSR04_Measure_h);
  xTaskCreate(MQTT_Publisher_Task, "MQTT_PUBLISH",4024,NULL,0,&mqttPublisher_h);
}
