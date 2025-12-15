#pragma once

#include <cstdint>
#include "driver/i2c.h"
#include "esp_err.h"

// Default I2C Address
#define SI4713_I2C_ADDR_LOW  0x11
#define SI4713_I2C_ADDR_HIGH 0x63

class Si4713 {
public:
    // Constructor requires the I2C port you want to use (e.g., I2C_NUM_0)
    Si4713(i2c_port_t i2c_num, uint8_t address = SI4713_I2C_ADDR_LOW);

    // Initialize the I2C driver and the chip
    esp_err_t init(int sda_pin, int scl_pin);

    // Commands
    esp_err_t setFrequency(uint16_t freqKHz);
    esp_err_t setTxPower(uint8_t dbuV);
    
    // Low level helpers (exposed if you need to send raw commands)
    esp_err_t sendCommand(uint8_t cmd, const uint8_t* args, size_t argCount);
    uint8_t   readStatus();

private:
    i2c_port_t _i2c_num;
    uint8_t    _address;

    void      waitForCTS();
    esp_err_t setProperty(uint16_t propID, uint16_t value);
};