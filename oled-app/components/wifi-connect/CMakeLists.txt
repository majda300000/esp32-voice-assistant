idf_component_register(SRCS "wifi_connect.c"
                       INCLUDE_DIRS "." "${CMAKE_SOURCE_DIR}"
                       REQUIRES esp_wifi nvs_flash)