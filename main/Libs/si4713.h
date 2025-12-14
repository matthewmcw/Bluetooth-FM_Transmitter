#pragma once
#include "driver/i2c.h"
#include "esp_log.h"

class si4713 {
private:
    i2c_port_t _i2c_port;
    uint8_t _address;

public:
    si4713(i2c_port_t i2c_port = I2C_NUM_0, uint8_t address = 0x63) 
        : _i2c_port(i2c_port), _address(address) {}

    bool begin() {
        // TODO: Initialize I2C
        return true;
    }
};