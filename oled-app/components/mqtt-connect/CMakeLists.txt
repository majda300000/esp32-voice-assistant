idf_component_register(SRCS "mqtt_connect.c"
                       INCLUDE_DIRS "." "${CMAKE_SOURCE_DIR}"
                       REQUIRES esp_wifi nvs_flash mqtt display)