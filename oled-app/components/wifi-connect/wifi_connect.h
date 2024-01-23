#ifndef WIFI_CONNECT_H
#define WIFI_CONNECT_H

#include <stdbool.h>

bool wifi_init(void);
void initialize_sntp(bool wifi_successfully_initialized);
char *get_sntp_time(void);

#endif