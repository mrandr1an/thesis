#include "hcsr04.h"
#include "freertos/idf_additions.h"
#include <esp_log.h>
#include <esp_attr.h>
#include <stdio.h>
#include <ultrasonic.h>

TaskHandle_t HCSR04_Init_h;
TaskHandle_t HCSR04_Measure_h;

void HCSR04_init_Task(void *pvParameters)
{
  ultrasonic_sensor_t sensor = {
    .trigger_pin = CONFIG_TRIGGER_PIN,
    .echo_pin = CONFIG_ECHO_PIN,
  };

  ultrasonic_init(&sensor);

  queue = xQueueCreate(1,sizeof(ultrasonic_sensor_t));

  xQueueSend(queue,(void*) &sensor, (TickType_t) 0);

  vTaskDelete(NULL);  // Delete the task after initialization
}

QueueHandle_t distance_queue = NULL;

void HCSR04_measure_Task(void *pvParameters)
{
  ultrasonic_sensor_t sensor;
    // Wait to receive the sensor structure from the queue
    if (xQueueReceive(queue, &sensor, portMAX_DELAY) == pdPASS) {
        ESP_LOGI(HCSR04_TAG, "Received sensor data");
    } else {
        ESP_LOGE(HCSR04_TAG, "Failed to receive sensor from queue");
    }

    float distance;
    distance_queue = xQueueCreate(1,sizeof(float));

    while (1) {
      esp_err_t res = ultrasonic_measure(&sensor,500,&distance);
      if(res != ESP_OK)
      {
	ESP_LOGW(HCSR04_TAG, "Error measuring distance: %d.", res);
            switch (res)
            {
                case ESP_ERR_ULTRASONIC_PING:
		    ESP_LOGE(HCSR04_TAG,"Cannot ping (device is in invalid state).");
                    break;
                case ESP_ERR_ULTRASONIC_PING_TIMEOUT:
		    ESP_LOGE(HCSR04_TAG,"Ping timeout (no device found).");
                    break;
                case ESP_ERR_ULTRASONIC_ECHO_TIMEOUT:
		    ESP_LOGE(HCSR04_TAG,"Echo timeout (i.e. distance too big).");
                    break;
                default:
		  ESP_LOGE(HCSR04_TAG,"Unresolved Error: %s.",esp_err_to_name(res));
            }
      } else
	{
	  ESP_LOGI(HCSR04_TAG, "Distance measured: %0.04f", distance*100);
	  xQueueSend(distance_queue,(void*) &distance, (TickType_t) 0);
	}
      vTaskDelay(pdMS_TO_TICKS(500)); //Delay 500ms
    }
}
