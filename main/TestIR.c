#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "src/wifi.h"
#include "src/MLX90640_I2C_Driver.h"
#include "src/MLX90640_API.h"

#include "src/i2c_general.h"


static const char *TAG = "MAIN_TEST";

i2c_master_bus_handle_t main_handle = NULL;

#define MLXADDR 0x33

void app_main(void)
{
    ESP_ERROR_CHECK(I2C_Init());
    MLX90640_I2CInit();
	nvs_init();
	wifi_connect();
	init_udp_socket();


    int status;
	int subpage;
	paramsMLX90640 mlx90640;
	float emissivity = 0.95;
	float tr = 23.15;
    static uint16_t eeMLX[832];	
	uint16_t frame[834]; /*= (834*sizeof(uint16_t));*/

	float *image_buffer1 = malloc(768*sizeof(float));
	float *image_buffer2 = malloc(768*sizeof(float));
	float *image		 = malloc(768*sizeof(float));


    MLX90640_SetResolution(MLXADDR, 0x00);
	MLX90640_SetRefreshRate(MLXADDR, 0x01);
	printf("Rozdzielczość: %d\n", MLX90640_GetCurResolution(MLXADDR));
	printf("FPS: %d\n", MLX90640_GetRefreshRate(MLXADDR));
	
	status = MLX90640_DumpEE(MLXADDR, eeMLX);
	ESP_LOGI(TAG, "DumpEE %d", status);
	
	status = MLX90640_ExtractParameters(eeMLX, &mlx90640);
	ESP_LOGI(TAG, "ExtractParameters %d", status);
	
	status = MLX90640_TriggerMeasurement (MLXADDR);
	ESP_LOGI(TAG, "TriggerMeasurement %d", status);
	


	while (1) {

		status = MLX90640_GetFrameData(MLXADDR, frame);
		subpage = MLX90640_GetSubPageNumber(frame);
			if (subpage == 0) {
				MLX90640_CalculateTo(frame, &mlx90640, emissivity, tr, image_buffer1);
			} else {
				MLX90640_CalculateTo(frame, &mlx90640, emissivity, tr, image_buffer2);
			}
			ESP_LOGI(TAG, "Subpage: %d", subpage);

			status = MLX90640_GetFrameData(MLXADDR, frame);
			subpage = MLX90640_GetSubPageNumber(frame);
			if (subpage == 1) {
				MLX90640_CalculateTo(frame, &mlx90640, emissivity, tr, image_buffer2);
			} else {
				MLX90640_CalculateTo(frame, &mlx90640, emissivity, tr, image_buffer1);
			}
			ESP_LOGI(TAG, "Subpage: %d", subpage);
			
			for (int row = 0; row < 24; row++) {
				for (int col = 0; col < 32; col++) {
					int index = row * 32 + col;
					if ((row + col) % 2 == 0) {
						image[index] = image_buffer1[index];
					} else {
						image[index] = image_buffer2[index];
					}
				}
			}

		send_udp_image(image);
		vTaskDelay(1000 / portTICK_PERIOD_MS);
	}
	
}
