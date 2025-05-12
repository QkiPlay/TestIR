#ifndef WIFI_H
#define WIFI_H

#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"
#include "esp_system.h"
#include "esp_wifi.h"
#include "esp_event.h"
#include "esp_log.h"
#include "nvs_flash.h"


#include <stdio.h>

#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>


#define WIFI_SUCCESS 1 << 0
#define WIFI_FAILURE 1 << 1
#define MAX_FAILURES 3
#define SSID
#define PWD

typedef struct{
	char tekst[50];
} Data_s;
typedef struct{
	float image[768];
} Image_p;

void nvs_init();
void wifi_connect();
void init_udp_socket();
void send_udp_data(Data_s *data);
void send_udp_image(float *image_data);

#endif