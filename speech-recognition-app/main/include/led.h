#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"
#include "sdkconfig.h"

#ifndef _LED_H_
#define _LED_H_

void led_init();
void led_on();
void led_off();
void ledBlinkTask();

#endif