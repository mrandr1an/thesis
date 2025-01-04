idf_component_register(SRCS "main.c" "wifi.c" "webserver.c" "hcsr04.c" "mqtt.c" "lm35.c"
                    INCLUDE_DIRS "")
spiffs_create_partition_image(storage ../web/ FLASH_IN_PROJECT)
