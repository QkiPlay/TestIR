#ifndef I2C_GENERAL_H
#define I2C_GENERAL_H

// Include necessary standard or project-specific headers
#include <stdint.h>
#include "esp_err.h"
#include "driver/i2c_master.h"
#include "esp_err.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"



// Macro definitions
#define I2C_SUCCESS 0
#define I2C_ERROR   -1

// Function declarations
esp_err_t I2C_Init(void);
esp_err_t I2C_Deinit(void);
esp_err_t I2C_set_pins(uint8_t scl, uint8_t sda);

/*TO DO*/
esp_err_t I2C_Write(uint8_t deviceAddress, const uint8_t *data, uint16_t length);
esp_err_t I2C_Read(uint8_t deviceAddress, uint8_t *data, uint16_t length);

#endif // I2C_GENERAL_H