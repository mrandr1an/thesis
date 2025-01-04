#pragma once

#include "freertos/idf_additions.h"
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>

#define HCSR04_TAG "HCSR04"

extern TaskHandle_t HCSR04_Init_h;
extern TaskHandle_t HCSR04_Measure_h;

static QueueHandle_t queue;
extern QueueHandle_t distance_queue;

void HCSR04_init_Task(void *pvParameters);
void HCSR04_measure_Task(void *pvParameters);


  
