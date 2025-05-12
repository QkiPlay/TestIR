#include "wifi.h"

#define PORT 3333
#define DEST_IP "192.168.50.58" // Define the destination IP address

#define SSID "SSID"
#define PWD "PWD"

static const char *TAG = "WiFi";
static EventGroupHandle_t wifi_event_group;
static int s_retry_num = 0;

static int sockt = -1;

static void wifi_event_handler(void* arg, esp_event_base_t event_base,
								int32_t event_id, void* event_data){
if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_START)
	{
		ESP_LOGI(TAG, "Connecting to AP...");
		esp_wifi_connect();
	} else if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_DISCONNECTED)
	{
		if (s_retry_num < MAX_FAILURES)
		{
			ESP_LOGI(TAG, "Reconnecting to AP...");
			esp_wifi_connect();
			s_retry_num++;
		} else {
			xEventGroupSetBits(wifi_event_group, WIFI_FAILURE);
		}
	}
}

static void ip_event_handler(void* arg, esp_event_base_t event_base,
                                int32_t event_id, void* event_data){
	if (event_base == IP_EVENT && event_id == IP_EVENT_STA_GOT_IP)
	{
        ip_event_got_ip_t* event = (ip_event_got_ip_t*) event_data;
        ESP_LOGI(TAG, "STA IP: " IPSTR, IP2STR(&event->ip_info.ip));
        s_retry_num = 0;
        xEventGroupSetBits(wifi_event_group, WIFI_SUCCESS);
    }

}

void nvs_init(){
	esp_err_t ret = nvs_flash_init();
	if(ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND){
		ESP_ERROR_CHECK(nvs_flash_erase());
		ret = nvs_flash_init();
	}
	ESP_ERROR_CHECK(ret);
};

void wifi_connect(){

	ESP_ERROR_CHECK(esp_netif_init());

	ESP_ERROR_CHECK(esp_event_loop_create_default());

	esp_netif_create_default_wifi_sta();

	wifi_init_config_t wifi_default_cfg = WIFI_INIT_CONFIG_DEFAULT();

	ESP_ERROR_CHECK(esp_wifi_init(&wifi_default_cfg));
	
	wifi_event_group = xEventGroupCreate();

	esp_event_handler_instance_t wifi_handler_event_instance;
	ESP_ERROR_CHECK(esp_event_handler_instance_register(WIFI_EVENT,
														ESP_EVENT_ANY_ID,
														&wifi_event_handler,
														NULL,
														&wifi_handler_event_instance));

	esp_event_handler_instance_t got_ip_event_instance;
	ESP_ERROR_CHECK(esp_event_handler_instance_register(IP_EVENT,
														IP_EVENT_STA_GOT_IP,
														&ip_event_handler,
														NULL,
														&got_ip_event_instance));
	
	wifi_config_t wifi_cfg = {
			.sta = {
				.ssid = SSID,
				.password = PWD,
				.threshold.authmode = WIFI_AUTH_WPA2_PSK,
				.pmf_cfg = {
					.capable = true,
					.required = false
				},
			},
	};

	ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));
	
	ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_STA, &wifi_cfg));
	
	ESP_ERROR_CHECK(esp_wifi_start());

	ESP_LOGI(TAG, "STA Init complete");

	EventBits_t bits = xEventGroupWaitBits(wifi_event_group,
		WIFI_SUCCESS | WIFI_FAILURE,
		pdFALSE,
		pdFALSE,
		portMAX_DELAY);

    if (bits & WIFI_SUCCESS) {
        ESP_LOGI(TAG, "Connected to ap");
    } else if (bits & WIFI_FAILURE) {
        ESP_LOGW(TAG, "Failed to connect to ap");
    } else {
        ESP_LOGE(TAG, "UNEXPECTED EVENT");
    }

    ESP_ERROR_CHECK(esp_event_handler_instance_unregister(IP_EVENT, IP_EVENT_STA_GOT_IP, got_ip_event_instance));
    ESP_ERROR_CHECK(esp_event_handler_instance_unregister(WIFI_EVENT, ESP_EVENT_ANY_ID, wifi_handler_event_instance));
    vEventGroupDelete(wifi_event_group);
}


void init_udp_socket() {
    struct sockaddr_in server_addr;

    // Create a socket
    sockt = socket(AF_INET, SOCK_DGRAM, 0);

    // Set up the server address
    memset(&server_addr, 0, sizeof(server_addr)); // Clear the structure
    server_addr.sin_family = AF_INET; // Use IPv4
    server_addr.sin_addr.s_addr = INADDR_ANY; // Listen on all interfaces
    server_addr.sin_port = htons(PORT); // Set the port number in network byte order

    // Bind the socket to the address and port
    bind(sockt, (struct sockaddr *)&server_addr, sizeof(server_addr));

    // Log success
    ESP_LOGI("UDP", "UDP socket initialized on port %d", PORT);
}



// Function to send data through UDP
void send_udp_data(Data_s *data) {
    struct sockaddr_in dest_addr;

    // Set up the destination address structure
    memset(&dest_addr, 0, sizeof(dest_addr)); // Clear the structure
    dest_addr.sin_family = AF_INET; // Use IPv4
    dest_addr.sin_port = htons(PORT); // Set the port number in network byte order
    inet_pton(AF_INET, // IPv4
    		  DEST_IP, //
    		  &dest_addr.sin_addr); // Convert IP address from text to binary form

    // Send the data
    sendto(sockt, data, sizeof(Data_s), 0, (struct sockaddr *)&dest_addr, sizeof(dest_addr));

}

void send_udp_image(float *image_data) {
    Image_p image_packet;
    memset(&image_packet, 0, sizeof(Image_p));

    memcpy(image_packet.image, image_data, 768 * sizeof(float));

    struct sockaddr_in dest_addr;

    // Set up the destination address structure
    memset(&dest_addr, 0, sizeof(dest_addr)); // Clear the structure
    dest_addr.sin_family = AF_INET; // Use IPv4
    dest_addr.sin_port = htons(PORT); // Set the port number in network byte order
    inet_pton(AF_INET, DEST_IP, &dest_addr.sin_addr); // Convert IP address from text to binary form

    // Send the data
    sendto(sockt, &image_packet, sizeof(Image_p), 0, (struct sockaddr *)&dest_addr, sizeof(dest_addr));

	/*Debug, wyświetla wysłane dane.
    for (int y = 0; y < 24; ++y) {
        for (int x = 0; x < 32; ++x) {
            printf("%6.2f ", image_packet.image[y * 32 + x]); // Print each float with 2 decimal places
        }
        printf("\n"); // Newline after each row
    }
	*/
}