
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "speech_commands_action.h"
#include "led.h"
#include "mqtt_client.h"
#include "mqtt_connect.h"

/*
commands:
id (0): "Turn on the light"
id (1): "Turn off the light"
id (2): "Upali svijetlo"
id (3): "Ugasi svjetlo"
id (4): "Say Hello"
id (5): "What time is it?"
id (6): "Show me the temperature"
id (7): "I'm sending you an image!"
*/

void speech_commands_action(int command_id)
{
    printf("Commands ID: %d.\n", command_id);
    switch (command_id)
    {
    case 0:
        printf("\n-TURN ON-\n");
        my_publish(command_id);
        led_on();
        break;
    case 1:
        printf("\n-TURN OFF-\n");
        my_publish(1);
        led_off();
        break;
    case 2:
        printf("\n-PALIM LEDICU-\n");
        my_publish(2);
        led_on();
        break;
    case 3:
        printf("\n-GASIM LEDICU-\n");
        my_publish(3);
        led_off();
        break;
    default:
        my_publish(command_id);
        break;
    }
}
