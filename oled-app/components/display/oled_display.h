#ifndef OLED_DISPLAY_H
#define OLED_DISPLAY_H

#include <stddef.h>
#include <stdint.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"

typedef enum
{
    standing,
    sitting,
    laughing
} kuromi_pic;

typedef struct Kuromi {
    bool kuromi_on_right;
    char kuromi_text[25];
    uint8_t buff[1024];
    uint8_t buff_bg[1024];
} Kuromi;

void oled_init(void);
void kuromi_speak(const char *text, size_t len);
void kuromi_show_text(const char *text, size_t len);
void kuromi_send_pic(uint8_t *buffer);
void kuromi_decode_and_send_pic(kuromi_pic pose);
void kuromi_fade(void);
void kuromi_return(void);


#endif