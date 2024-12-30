idf_component_register(SRCS "main.c" "wifi.c" "portal.c"
                       INCLUDE_DIRS ".")

spiffs_create_partition_image(storage ./web FLASH_IN_PROJECT)
