#include "led.h"

#define GPIO_NUM GPIO_NUM_22

int led_state = 0;

void led_init()
{
   gpio_config_t io_conf;
   io_conf.intr_type = (gpio_int_type_t)GPIO_PIN_INTR_DISABLE;
   io_conf.mode = GPIO_MODE_OUTPUT;
   io_conf.pull_up_en = (gpio_pullup_t)1;

   uint64_t pin = ((uint64_t)1 << GPIO_NUM);
   io_conf.pin_bit_mask = pin;
   gpio_config(&io_conf);
   gpio_set_level(GPIO_NUM, 0);
}

void led_on()
{
   gpio_set_level(GPIO_NUM, 1);
   led_state = 1;
}

void led_off()
{
   gpio_set_level(GPIO_NUM, 0);
   led_state = 0;
}

void ledBlinkTask(void *arg)
{
   int blink = led_state;
   for (int i = 0; i < 4; i++)
   {
      blink = 1 - blink;
      gpio_set_level(GPIO_NUM, blink);
      vTaskDelay(20);
   }
   gpio_set_level(GPIO_NUM, led_state);
   vTaskDelete(NULL);
}
