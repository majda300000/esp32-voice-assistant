idf_component_register(SRCS "app_main.c"
                    INCLUDE_DIRS "."
                    REQUIRES dht11 mqtt esp_wifi nvs_flash wifi-connect mqtt-connect display spiffs )

spiffs_create_partition_image(storage ../spiffs-images FLASH_IN_PROJECT)
