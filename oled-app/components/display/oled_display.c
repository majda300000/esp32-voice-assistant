#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "esp_spiffs.h"

#include "ssd1306.h"
#include "font8x8_basic.h"
#include "images.h"
#include "oled_display.h"
#include "kuromi_tasks.h"

#define CONFIG_SDA_GPIO 21
#define CONFIG_SCL_GPIO 22
#define CONFIG_RESET_GPIO 15
#define CONFIG_OFFSETX 2
#define BMP_TO_HEX_OFFSET 62

#define tag "OLED"

SSD1306_t dev;
FILE *file;


Kuromi kuromi = {
	.kuromi_on_right = false, 
	.kuromi_text = "", 
	.buff = {0}
	//.side_buff = {0},
};

void kuromi_laugh(void);
void kuromi_speak(const char *text, size_t len);


void oled_init(void)
{
	ESP_LOGI(tag, "CONFIG_SDA_GPIO=%d",CONFIG_SDA_GPIO);
	ESP_LOGI(tag, "CONFIG_SCL_GPIO=%d",CONFIG_SCL_GPIO);
	ESP_LOGI(tag, "CONFIG_RESET_GPIO=%d",CONFIG_RESET_GPIO);
	i2c_master_init(&dev, CONFIG_SDA_GPIO, CONFIG_SCL_GPIO, CONFIG_RESET_GPIO);

    ESP_LOGI(tag, "Panel is 128x64");
	ssd1306_init(&dev, 128, 64);

    ssd1306_clear_screen(&dev, false);
	ssd1306_contrast(&dev, 0xff);
	kuromi_init();
	
	kuromi_decode_and_send_pic(standing);
	ssd1306_get_buffer(&dev, kuromi.buff);
}

void kuromi_laugh(void)
{
	ssd1306_bitmaps(&dev, 0, 0, image_data_kuromi, 128, 64, false);
	ssd1306_bitmaps(&dev, 0, 0, image_data_kuromi_laugh, 128, 64, false);
	ssd1306_bitmaps(&dev, 0, 0, image_data_kuromi, 128, 64, false);
	ssd1306_bitmaps(&dev, 0, 0, image_data_kuromi_laugh, 128, 64, false);
	ssd1306_bitmaps(&dev, 0, 0, image_data_kuromi, 128, 64, false);
	ssd1306_bitmaps(&dev, 0, 0, image_data_kuromi_standing, 128, 64, false);
}

void kuromi_show_text(const char *text, size_t len){
	for(int i =3; i<6;i++){
			ssd1306_clear_line(&dev, i, true);
		}
	ssd1306_set_buffer(&dev, kuromi.buff);
	ssd1306_show_buffer(&dev);
	strcpy(kuromi.kuromi_text,text);
	if (len <= 6){
		char text_display[50];
		sprintf(text_display, " %s", text);
		ssd1306_display_text(&dev, 3, text_display, len+1, true);
	} else if (len <= 15) {
		char text_display[50];
		sprintf(text_display, " %.*s",6, text);
		ssd1306_display_text(&dev, 3, text_display, 6+1, true);
		text+=6;
		sprintf(text_display, " %s", text);
		ssd1306_display_text(&dev, 4, text_display, len-6, true);
	} else if (len <= 25) {
		char text_display[50];
		sprintf(text_display, " %.*s",6, text);
		ssd1306_display_text(&dev, 3, text_display, 6+1, true);
		text+=6;
		sprintf(text_display, " %.*s",9, text);
		ssd1306_display_text(&dev, 4, text_display, 9, true);
		text+=9;
		sprintf(text_display, " %s", text);
		ssd1306_display_text(&dev, 5, text_display, len-6-9-1, true);
	}
}

void kuromi_fade(void){
	//ssd1306_get_buffer(&dev, kuromi.buff);	
	ssd1306_fadeout(&dev);
}

void kuromi_speak(const char *text, size_t len)
{
	if(!kuromi.kuromi_on_right){
		for(int i =0; i<29;i++){
			ssd1306_wrap_arround(&dev, SCROLL_RIGHT, 0, 7,0);
			kuromi.kuromi_on_right = true;
		}
		ssd1306_get_buffer(&dev, kuromi.buff);		
	} else {
		for(int i =3; i<6;i++){
			ssd1306_clear_line(&dev, i, true);
		}
		ssd1306_set_buffer(&dev, kuromi.buff);
		ssd1306_show_buffer(&dev);
		//kuromi_send_pic(kuromi.buff);
	}
	strcpy(kuromi.kuromi_text,text);
	if (len <= 6){
		char text_display[50];
		sprintf(text_display, " %s", text);
		ssd1306_display_text(&dev, 3, text_display, len+1, true);
	} else if (len <= 15) {
		char text_display[50];
		sprintf(text_display, " %.*s",6, text);
		ssd1306_display_text(&dev, 3, text_display, 6+1, true);
		text+=6;
		sprintf(text_display, " %s", text);
		ssd1306_display_text(&dev, 4, text_display, len-6, true);
	} else if (len <= 25) {
		char text_display[50];
		sprintf(text_display, " %.*s",6, text);
		ssd1306_display_text(&dev, 3, text_display, 6+1, true);
		text+=6;
		sprintf(text_display, " %.*s",9, text);
		ssd1306_display_text(&dev, 4, text_display, 9, true);
		text+=9;
		sprintf(text_display, " %s", text);
		ssd1306_display_text(&dev, 5, text_display, len-6-9-1, true);
	}
}

void kuromi_send_pic(uint8_t *buffer){
	ssd1306_bitmaps(&dev, 0, 0, buffer, 128, 64, false);
}

void kuromi_return(){
	if(kuromi.kuromi_on_right){
		for(int i =3; i<6;i++){
			ssd1306_clear_line(&dev, i, true);
		}
		ssd1306_set_buffer(&dev, kuromi.buff);
		ssd1306_show_buffer(&dev);
		for(int i =0; i<29;i++){
			ssd1306_wrap_arround(&dev, SCROLL_LEFT, 0, 7,0);
			kuromi.kuromi_on_right = false;
		}
	ssd1306_get_buffer(&dev, kuromi.buff);
	}
}

void kuromi_decode_and_send_pic(kuromi_pic pose){

	switch(pose){
		case standing:
			file = fopen("/spiffs/kuromi-standing.bmp", "r");
			break;
		case sitting:
			file = fopen("/spiffs/kuromi-sitting.bmp", "r");
			break;
		case laughing:
			file = fopen("/spiffs/kuromi-laugh.bmp", "r");
			break;
		default:
			file = fopen("/spiffs/kuromi-standing.bmp", "r");
			ESP_LOGI(tag, "That kuromi pose does not exist");
	}

    if(file == NULL)
    {
        ESP_LOGE(tag,"File does not exist!");
    }
	else 
    {
		char c = '0';
		int i =0;
		do{
			c = fgetc(file);
			//printf("%hx ", c);
			if(i>=BMP_TO_HEX_OFFSET){
				kuromi.buff[i-BMP_TO_HEX_OFFSET] = c;
				//printf("%hx ", c);
			}
			if(i>1023){
				kuromi.buff[i-BMP_TO_HEX_OFFSET] = 0xff;
			}
			i++;
		} while(i!=1024+BMP_TO_HEX_OFFSET);

		kuromi_send_pic(kuromi.buff);
		fclose(file);
    }
    //esp_vfs_spiffs_unregister(NULL);
}

		// if(!kuromi.kuromi_on_right){
		// 	kuromi_send_pic(kuromi.buff);

		// } else {
		// 	for(int i =3; i<6;i++){
		// 		ssd1306_clear_line(&dev, i, true);
		// 	}
		// 	kuromi_send_pic(kuromi.buff);
		// 	for(int i =0; i<29;i++){
		// 		ssd1306_wrap_arround(&dev, SCROLL_LEFT, 0, 7,0);
		// 	}
		// 	kuromi.kuromi_on_right = true;
		// 	for(int i =0; i<29;i++){
		// 		ssd1306_wrap_arround(&dev, SCROLL_RIGHT, 0, 7,0);
		// 	}
		// 	ssd1306_get_buffer(&dev, kuromi.buff);
		// 	kuromi_speak(kuromi.kuromi_text, strlen(kuromi.kuromi_text));
		// }