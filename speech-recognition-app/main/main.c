
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>
#include <stddef.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "esp_system.h"
#include "sdkconfig.h"

#include "esp_wifi.h"
#include "nvs_flash.h"
#include "esp_event.h"

#include "MediaHal.h"
#include "esp_wn_iface.h"
#include "esp_wn_models.h"
#include "esp_mn_iface.h"
#include "esp_mn_models.h"
#include "recsrc.h"
#include "driver/i2s.h"
#include "ringbuf.h"
#include "speech_commands_action.h"
#include "led.h"

#include "freertos/FreeRTOS.h"
#include "freertos/event_groups.h"
#include "freertos/task.h"
#include "freertos/semphr.h"
#include "freertos/queue.h"
#include "freertos/timers.h"
#include "esp_log.h"
#include "mqtt_client.h"
#include "mqtt_connect.h"
#include "wifi_connect.h"
#include "wake_button.h"

// WakeNet
static const esp_wn_iface_t *wakenet = &WAKENET_MODEL;
static const model_coeff_getter_t *model_coeff_getter = &WAKENET_COEFF;
// MultiNet
static const esp_mn_iface_t *multinet = &MULTINET_MODEL;
model_iface_data_t *model_data_mn = NULL;

struct RingBuf *rec_rb = NULL;
struct RingBuf *ns_rb = NULL;
struct RingBuf *agc_rb = NULL;

static const char *TAG = "MAIN";

void wakenetTask(void *arg)
{
    model_iface_data_t *model_data = arg;
    int frequency = wakenet->get_samp_rate(model_data);
    int audio_chunksize = wakenet->get_samp_chunksize(model_data);
    int chunk_num = multinet->get_samp_chunknum(model_data_mn);
    printf("chunk_num = %d\n", chunk_num);
    int16_t *buffer = malloc(audio_chunksize * sizeof(int16_t));
    assert(buffer);
    int chunks = 0;
    int mn_chunks = 0;
    bool detect_flag = 0;

    while (1)
    {
        rb_read(agc_rb, (uint8_t *)buffer, audio_chunksize * sizeof(int16_t), portMAX_DELAY);
        if (detect_flag == 0)
        {
            int r = wakenet->detect(model_data, buffer);
            int button = wake_button();
            if (r || button)
            {
                if (r)
                {
                    float ms = (chunks * audio_chunksize * 1000.0) / frequency;
                    printf("%.2f: %s DETECTED.\n", (float)ms / 1000.0, wakenet->get_word_name(model_data, r));
                }
                else
                {
                    printf("VOL+ BUTTON PRESSED, DEVICE WOKEN UP\n");
                }
                detect_flag = 1;
                printf("-----------------LISTENING-----------------\n\n");
                xTaskCreatePinnedToCore(&ledBlinkTask, "blink", 2 * 1024, NULL, 8, NULL, 1);
            }
        }
        else
        {
            int command_id = multinet->detect(model_data_mn, buffer);
            mn_chunks++;
            if (mn_chunks == chunk_num || command_id > -1)
            {
                mn_chunks = 0;
                detect_flag = 0;
                if (command_id > -1)
                {
                    speech_commands_action(command_id);
                }
                else
                {
                    printf("can not recognize any speech commands\n");
                }

                printf("\n-----------awaits to be waken up-----------\n");
            }
        }
        chunks++;
    }
    vTaskDelete(NULL);
}

void app_main()
{
    led_init();
    wake_button_init();
    codec_init();
    rec_rb = rb_init(BUFFER_PROCESS, 8 * 1024, 1, NULL);
    ns_rb = rb_init(BUFFER_PROCESS, 8 * 1024, 1, NULL);
    agc_rb = rb_init(BUFFER_PROCESS, 8 * 1024, 1, NULL);

    model_iface_data_t *model_data = wakenet->create(model_coeff_getter, DET_MODE_90);
    model_data_mn = multinet->create(&MULTINET_COEFF, 6000);

    esp_log_level_set("*", ESP_LOG_INFO);
    esp_log_level_set("MQTT_CLIENT", ESP_LOG_VERBOSE);
    esp_log_level_set("TRANSPORT_TCP", ESP_LOG_VERBOSE);
    esp_log_level_set("TRANSPORT_SSL", ESP_LOG_VERBOSE);
    esp_log_level_set("TRANSPORT", ESP_LOG_VERBOSE);
    esp_log_level_set("OUTBOX", ESP_LOG_VERBOSE);

    int wifi_connected = 0;
    nvs_flash_init();
    wifi_connected = wifi_init_sta();
    mqtt_app_start(wifi_connected);

    xTaskCreatePinnedToCore(&recsrcTask, "rec", 2 * 1024, NULL, 8, NULL, 1);
    xTaskCreatePinnedToCore(&nsTask, "ns", 2 * 1024, NULL, 8, NULL, 1);
    xTaskCreatePinnedToCore(&agcTask, "agc", 2 * 1024, NULL, 8, NULL, 1);
    xTaskCreatePinnedToCore(&wakenetTask, "wakenet", 2 * 1024, (void *)model_data, 5, NULL, 1);

    printf("-----------awaits to be waken up-----------\n");
}
