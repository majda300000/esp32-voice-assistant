#include <stdio.h>
#include <stdint.h>
#include <stddef.h>
#include <string.h>
#include <stdbool.h>
#include "esp_wifi.h"
#include "esp_system.h"
#include "nvs_flash.h"
#include "esp_event.h"
#include "esp_netif.h"

#include "freertos/FreeRTOS.h"
#include "freertos/event_groups.h"
#include "freertos/task.h"
#include "freertos/semphr.h"
#include "freertos/queue.h"
#include "freertos/timers.h"

#include "lwip/sockets.h"
#include "lwip/dns.h"
#include "lwip/netdb.h"
#include "lwip/err.h"
#include "lwip/sys.h"

#include "esp_log.h"
#include "mqtt_client.h"
#include "oled_display.h"
#include "kuromi_tasks.h"

#include "projectconfig.h"


#define BROKER_URL CONFIG_MY_BROKER_URL

static const char *TAG = "MQTT";
esp_mqtt_client_handle_t client;
TaskHandle_t myTaskHandle = NULL;

static void mqtt_event_handler(void *handler_args, esp_event_base_t base, int32_t event_id, void *event_data)
{
    ESP_LOGD(TAG, "Event dispatched from event loop base=%s, event_id=%" PRIi32 "", base, event_id);
    esp_mqtt_event_handle_t event = event_data;
    client = event->client;
    char topic[20];
    char data[2500];

    switch ((esp_mqtt_event_id_t)event_id) {

        case MQTT_EVENT_CONNECTED:
            //esp_mqtt_client_publish(client, "Alooo", "caooooo", 0, 1, 0);
            ESP_LOGI(TAG, "MQTT_EVENT_CONNECTED");
            esp_mqtt_client_subscribe(client, "sunnyday", 0);
            esp_mqtt_client_subscribe(client, "fromLyraT", 0);
            esp_mqtt_client_subscribe(client, "picture", 0);
            break;

        case MQTT_EVENT_DISCONNECTED:
            ESP_LOGI(TAG, "MQTT_EVENT_DISCONNECTED");
            break;

        case MQTT_EVENT_SUBSCRIBED:
            break;

        case MQTT_EVENT_UNSUBSCRIBED:
            ESP_LOGI(TAG, "MQTT_EVENT_UNSUBSCRIBED, msg_id=%d", event->msg_id);
            break;

        case MQTT_EVENT_PUBLISHED:
            ESP_LOGI(TAG, "MQTT_EVENT_PUBLISHED, msg_id=%d", event->msg_id);
            break;

        case MQTT_EVENT_DATA:
            printf("BAREM TU?\n");
            //printf("PRIMLJENO = (%.*s) -> %.*s\r\n", event->topic_len, event->topic, event->data_len, event->data);
            sprintf(topic, "%.*s",  event->topic_len, event->topic);
            sprintf(data, "%.*s",  event->data_len, event->data);

            decode_command(topic, data, event->data_len);
            break;

        case MQTT_EVENT_ERROR:
            ESP_LOGI(TAG, "MQTT_EVENT_ERROR");
            if (event->error_handle->error_type == MQTT_ERROR_TYPE_TCP_TRANSPORT) {
                ESP_LOGI(TAG, "Last errno string (%s)", strerror(event->error_handle->esp_transport_sock_errno));
            }
            break;

        default:
            ESP_LOGI(TAG, "Other event id:%d", event->event_id);
            break;
    }
}


void mqtt_app_start(bool wifi_successfully_initialized)
{
    if(!wifi_successfully_initialized){
        ESP_LOGI(TAG, "not connecting to broker - wifi not connected");
        return;
    }
    esp_mqtt_client_config_t mqtt_cfg = {
        .broker.address.uri = BROKER_URL,
    };

    esp_mqtt_client_handle_t client = esp_mqtt_client_init(&mqtt_cfg);

    esp_mqtt_client_register_event(client, ESP_EVENT_ANY_ID, mqtt_event_handler, NULL);
    esp_mqtt_client_start(client);
    //xTaskCreate((void *)publish, "publish", 4096, NULL, 10, &myTaskHandle);
}

// void publish(void) {
//     int i = 0;
//     char mssg[30];
//     while (1) {
//         vTaskDelay(pdMS_TO_TICKS(10000));
//         snprintf(mssg, sizeof(mssg), "caoooo(%d)", i);
//         esp_mqtt_client_publish(client, "fromESP32", mssg, 0, 1, 0);
//         i++;
//     }
// }