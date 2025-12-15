#include "si4713.h"
#include <cstring> // for memcpy if needed
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

static const char* TAG = "Si4713";

// Command Definitions
#define CMD_POWER_UP      0x01
#define CMD_GET_REV       0x10
#define CMD_TX_TUNE_FREQ  0x30
#define CMD_TX_TUNE_POWER 0x31

Si4713::Si4713(i2c_port_t i2c_num, uint8_t address) 
    : _i2c_num(i2c_num), _address(address) {}

esp_err_t Si4713::init(int sda_pin, int scl_pin) {
    // 1. Configure I2C Parameter
    i2c_config_t conf = {};
    conf.mode = I2C_MODE_MASTER;
    conf.sda_io_num = sda_pin;
    conf.scl_io_num = scl_pin;
    conf.sda_pullup_en = GPIO_PULLUP_ENABLE;
    conf.scl_pullup_en = GPIO_PULLUP_ENABLE;
    conf.master.clk_speed = 100000; // 100kHz standard speed
    
    // 2. Install Driver
    esp_err_t err = i2c_param_config(_i2c_num, &conf);
    if (err != ESP_OK) return err;
    
    err = i2c_driver_install(_i2c_num, conf.mode, 0, 0, 0);
    if (err != ESP_OK) return err;

    ESP_LOGI(TAG, "I2C Initialized. Powering up Si4713...");
    
    // 3. Send Power Up Command
    // Arg1: 0x12 (CTS off, GPO2 off, Function=Transmit)
    // Arg2: 0x50 (Analog Audio Input)
    uint8_t args[] = {0x12, 0x50};
    
    // Note: We cannot verify CTS before the very first command, 
    // but we can assume the chip is ready after a hard reset.
    vTaskDelay(pdMS_TO_TICKS(10)); 
    
    // We send command manually here to avoid CTS check loop on unpowered chip
    err = i2c_master_write_to_device(_i2c_num, _address, 
                                     (const uint8_t*)"\x01\x12\x50", 3, 
                                     pdMS_TO_TICKS(100));

    if (err != ESP_OK) {
        ESP_LOGE(TAG, "I2C Write Failed during Power Up");
        return err;
    }

    // Wait for the chip to boot
    vTaskDelay(pdMS_TO_TICKS(100));
    
    return ESP_OK;
}

void Si4713::waitForCTS() {
    int retries = 0;
    uint8_t status = 0;
    
    while(retries < 200) {
        // Read 1 byte (Status byte)
        esp_err_t err = i2c_master_read_from_device(_i2c_num, _address, &status, 1, pdMS_TO_TICKS(10));
        
        if (err == ESP_OK) {
            // Check Bit 7 (CTS)
            if (status & 0x80) {
                return; // Ready
            }
        }
        
        // Wait 2ms before polling again
        vTaskDelay(pdMS_TO_TICKS(2));
        retries++;
    }
    ESP_LOGW(TAG, "CTS Timeout!");
}

esp_err_t Si4713::sendCommand(uint8_t cmd, const uint8_t* args, size_t argCount) {
    waitForCTS();

    // Prepare buffer: Command + Args
    // We allocate dynamic or use a small stack buffer. 
    // Max args is usually small (less than 8 bytes).
    uint8_t buffer[10];
    buffer[0] = cmd;
    if (argCount > 0 && args != nullptr) {
        memcpy(&buffer[1], args, argCount);
    }

    esp_err_t err = i2c_master_write_to_device(_i2c_num, _address, buffer, argCount + 1, pdMS_TO_TICKS(100));
    
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "I2C Write Error: %s", esp_err_to_name(err));
    }
    
    // Optional: Wait for CTS immediately after to ensure command process started
    waitForCTS();
    
    return err;
}

esp_err_t Si4713::setFrequency(uint16_t freqKHz) {
    uint8_t args[3];
    args[0] = 0x00; 
    args[1] = (uint8_t)(freqKHz >> 8);
    args[2] = (uint8_t)(freqKHz & 0xFF);
    return sendCommand(CMD_TX_TUNE_FREQ, args, 3);
}

esp_err_t Si4713::setTxPower(uint8_t dbuV) {
    uint8_t args[4] = {0x00, 0x00, dbuV, 0x00};
    return sendCommand(CMD_TX_TUNE_POWER, args, 4);
}