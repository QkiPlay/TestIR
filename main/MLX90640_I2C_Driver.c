#include "src/MLX90640_I2C_Driver.h"
#include "driver/i2c_master.h"
#include "esp_err.h"
#include "esp_log.h"

#include "src/i2c_general.h"


#define MLX90640_I2C_ADDR   0x33    // Default MLX90640 I2C address
#define FREQ 400000
static const char *TAG = "MLX90640";



extern i2c_master_bus_handle_t main_handle;
i2c_master_dev_handle_t mlx_handle = NULL;

i2c_device_config_t mlx_cfg = {
    .dev_addr_length = I2C_ADDR_BIT_LEN_7,
    .device_address = MLX90640_I2C_ADDR,
    .scl_speed_hz = FREQ,
    };



void MLX90640_I2CInit(void){
        ESP_ERROR_CHECK(i2c_master_bus_add_device(main_handle, &mlx_cfg, &mlx_handle));
    }
    
void MLX90640_I2CFreqSet(int freq){
        ESP_ERROR_CHECK(i2c_master_bus_rm_device(mlx_handle));
        mlx_cfg.scl_speed_hz = freq;
        ESP_ERROR_CHECK(i2c_master_bus_add_device(main_handle, &mlx_cfg, &mlx_handle));
};



int MLX90640_I2CGeneralReset(void) {	//Wygląda na to, że nie jest używana w ogóle
	uint8_t address = 0x00;  // General Call Address
	uint8_t data = 0x06;     // RESET command
	
	ESP_LOGI(TAG, "I2CGeneralReset");
	i2c_operation_job_t i2c_ops[] = {
		{ .command = I2C_MASTER_CMD_START },
		{ .command = I2C_MASTER_CMD_WRITE, .write = { .ack_check = false, .data = &address, .total_bytes = 1 } },
		{ .command = I2C_MASTER_CMD_WRITE, .write = { .ack_check = false, .data = &data, .total_bytes = 1 } },
		{ .command = I2C_MASTER_CMD_STOP }
	};
	return (int) i2c_master_execute_defined_operations(mlx_handle, i2c_ops, sizeof(i2c_ops) / sizeof(i2c_operation_job_t), 1000);
}



int MLX90640_I2CRead(uint8_t slaveAddr, uint16_t startAddress, uint16_t nMemAddressRead, uint16_t *data)
{
    /*ESP_LOGI(TAG, "I2CRead\tstartAddress: %d\tnMemAddressRead: %d\n", startAddress, nMemAddressRead);*/

    if (!mlx_handle) return -1;
    if (data == NULL) return -1;
    
    esp_err_t ret;

    uint8_t addr_buf[2] = {(startAddress >> 8), (startAddress & 0xFF)};
    uint8_t *readBuffer = (uint8_t *)malloc(nMemAddressRead*2);
    if (readBuffer == NULL) return -1;

    ret = i2c_master_transmit_receive(
        mlx_handle,
        addr_buf,
        sizeof(addr_buf),
        readBuffer,
        nMemAddressRead * 2,    
        -1
    );
    
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Transmit-receive failed: %s", esp_err_to_name(ret));
        free(readBuffer);
        return -1;
    }

    // Convert big-endian to little-endian
    for (uint16_t i = 0; i < nMemAddressRead; i++) {
        data[i] = (readBuffer[i*2] << 8) | readBuffer[i*2 + 1];
    }
    free(readBuffer);
    return 0;
}

int MLX90640_I2CWrite(uint8_t slaveAddr,uint16_t writeAddress, uint16_t data){
    if (!mlx_handle) {
        return -1;
    }

    esp_err_t ret;

	uint8_t buffer[4];
	buffer[0] = (writeAddress >> 8) & 0xFF;
	buffer[1] = writeAddress & 0xFF;
	buffer[2] = (data >> 8) & 0xFF;
	buffer[3] = data & 0xFF;

	// Transmisja danych
	ret = i2c_master_transmit(mlx_handle, buffer, sizeof(buffer), 1000);

	if (ret != ESP_OK) {
		return -1;
	}

	return 0;
}
    