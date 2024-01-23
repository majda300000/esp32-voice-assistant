#include "driver/gpio.h"

#define REC_BUTTON_PIN GPIO_NUM_39 // Replace with the actual GPIO number

void wake_button_init()
{
   gpio_pad_select_gpio(REC_BUTTON_PIN);
   gpio_set_direction(REC_BUTTON_PIN, GPIO_MODE_INPUT);
}

int wake_button()
{
   return gpio_get_level(REC_BUTTON_PIN) == 0; // Assuming the button is active low
}
