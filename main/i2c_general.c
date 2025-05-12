#include "i2c_general.h"
#include "driver/i2c_master.h"
#include "esp_err.h"
#include "esp_log.h"


#define I2C_TIMEOUT 1000 // Timeout in milliseconds

static const char *TAG = "I2C_GENERAL";

const int SCL_PIN = 18;
const int SDA_PIN = 17;

static int I2C_freq_khz = 400;

extern i2c_master_bus_handle_t main_handle;

static i2c_master_bus_config_t master_config = {
    .clk_source = I2C_CLK_SRC_RC_FAST,
    .i2c_port = -1,
    .scl_io_num = SCL_PIN,
    .sda_io_num = SDA_PIN,
    .glitch_ignore_cnt = 7,
    .flags.enable_internal_pullup = true,
};



/// @brief Inicjuje I2C z domyślnymi parametrami, SCL=18, SDA=17
/// @details Aby zmienić, użyć funckji i2c_set_pins() 
esp_err_t I2C_Init() {
    ESP_ERROR_CHECK(i2c_new_master_bus(&master_config, &main_handle));
    return ESP_OK;
};
/// @brief Usuwa główną szynę, zmienia piny, inicjuje magistralę od nowa
/// @param scl SCL pin
/// @param sda SDA pin
/// @return ESP_OK -> Piny takie same albo zmienione pomyślnie

esp_err_t I2C_set_pins(uint8_t scl, uint8_t sda) {
    
    if (scl == SCL_PIN && sda == SDA_PIN){
        return ESP_OK;
    }
    if (main_handle == NULL)
    {
        master_config.scl_io_num = scl;
        master_config.sda_io_num = sda;
        return ESP_OK;        
    }    
    if (main_handle != NULL){
        ESP_ERROR_CHECK(i2c_del_master_bus(main_handle));
        master_config.scl_io_num = scl;
        master_config.sda_io_num = sda;  
        ESP_ERROR_CHECK(i2c_new_master_bus(&master_config, &main_handle));
        return ESP_OK;
    }
    return ESP_FAIL;
};
/// @brief Deinicjuje I2C jeśli jakaś szyna istnieje
esp_err_t I2C_Deinit(){
    if(main_handle != NULL){
        ESP_ERROR_CHECK(i2c_del_master_bus(main_handle));
        return ESP_OK;
    }
    ESP_LOGI(TAG, "DUPA");
    return ESP_FAIL;
};