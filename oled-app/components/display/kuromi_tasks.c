#include <string.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/semphr.h"

#include "oled_display.h"
#include "wifi_connect.h"
#include "ssd1306.h"
#include "dht.h"

#define COMMAND_1 "Say Hello"
#define COMMAND_2 "What time is it?"
#define COMMAND_3 "Show me the temperature"

#define TOPIC_1 "fromLyraT"
#define TOPIC_2 "picture"


static const char *TAG = "KUROMI_TASKS";
uint8_t buffer[1024] = {0};
TaskHandle_t waitHandle = NULL;
SemaphoreHandle_t wait_mutex = NULL;
int wait_secs = 0;

kuromi_pic pose = standing;
int kuromi_state = 0;       //0 is kuromi is in the middle, 1 if kuromi is showing something

void start_or_reset_wait_task(int secs);
kuromi_pic kuromi_state_shuffle(kuromi_pic pose);
void kuromi_wait(void *pvParameters) ;

void kuromi_init(){
    pose = standing;
    kuromi_state = 0;
    wait_mutex = xSemaphoreCreateMutex();
}

void decode_command(const char *topic, char *command, size_t len)
{
    if(strcmp(topic, TOPIC_1) == 0){

        start_or_reset_wait_task(7);
        if (strcmp(command, COMMAND_1) == 0)
        {
            xSemaphoreTake( wait_mutex, portMAX_DELAY );
            printf("tu?\n");
            if(kuromi_state == 0){
                kuromi_speak("Hello!",6);
            } else {
                kuromi_show_text("Hello!",6);
            }
            kuromi_state = 1;
            xSemaphoreGive( wait_mutex );
        } 
        else if (strcmp(command, COMMAND_2) == 0) 
        {
            xSemaphoreTake( wait_mutex, portMAX_DELAY );
            char *time;
            time = get_sntp_time();
            char message[40];
            sprintf(message, "It's  %.*s    o'clock!", 5, time);
            if(kuromi_state == 0){
                kuromi_speak(message, 24);
            } else {
                kuromi_show_text(message, 24);
            }
            kuromi_state = 1;
            xSemaphoreGive( wait_mutex );
        } else if (strcmp(command, COMMAND_3) == 0) 
        {
            xSemaphoreTake( wait_mutex, portMAX_DELAY );
            int16_t humidity, temperature;
            dht_read_data(0, 4, &humidity, &temperature);
            char message[40];
            sprintf(message, "It's  %d deg.  Celsius!", temperature/10);
            if(kuromi_state == 0){
                kuromi_speak(message, 25);
            } else {
                kuromi_show_text(message, 25);
            }
            kuromi_state = 1;
            xSemaphoreGive( wait_mutex );
        }
    } else if(strcmp(topic, TOPIC_2) == 0){

        
        int k;
        int j;
        char command_byte[2] = {0};
        int part = command[0] - '0';
        command += 2;
        switch (part){
        case 1:
            k = 0;
            j = 680/2;
            break;
        case 2:
            k = 680/2;
            j = 1360/2;
            break;
        default:
            k = 1360/2;
            j = 2048/2;
            break;
        }
        for(int i = k; i < j; i++){
            strncpy(command_byte, command, 2);
            command += 2;
            buffer[i] = (int)strtol(command_byte, NULL, 16);
            //printf("(%d)%s  %d\n", i, command_byte, buffer[i]);
            
        }
        if(part == 3){
            xSemaphoreTake( wait_mutex, portMAX_DELAY );
            kuromi_state = 1;
            if(kuromi_state == 0){
                kuromi_show_text("Got   the pic!", 15);
            } else {
                kuromi_speak("Got   the pic!", 15);
            }
            vTaskDelay(3);
            kuromi_fade();
            kuromi_send_pic(buffer);
            start_or_reset_wait_task(10);
            xSemaphoreGive( wait_mutex );
        }
    } else {
        return;
    }
}

kuromi_pic kuromi_state_shuffle(kuromi_pic pose){
    switch(pose){
        case standing:
            return sitting;
        case sitting:
            return laughing;
        case laughing:
            return standing;
        default:
            return standing;
    }
}

void start_or_reset_wait_task(int secs){
    if (waitHandle != NULL) {      //reset counter if counter active 
        printf("handle mi nije null");
            vTaskDelete(waitHandle);
        }
    wait_secs = secs;
    printf("tu\n");
    xTaskCreate(kuromi_wait, "waiting task", 4096, &secs, 1, &waitHandle); //start counter
}

void kuromi_wait(void *pvParameters) {

    vTaskDelay(pdMS_TO_TICKS(wait_secs*1000));
    xSemaphoreTake( wait_mutex, portMAX_DELAY );
    printf("i zauzeo mutex\n");
    kuromi_return();
    pose = kuromi_state_shuffle(pose);
    printf("%d", pose);
    kuromi_decode_and_send_pic(pose);
    kuromi_state = 0;
    printf("izlazim\n");
    waitHandle = NULL;
    xSemaphoreGive( wait_mutex );
    vTaskDelete(NULL);
}
