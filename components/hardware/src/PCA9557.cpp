#include "PCA9557.hpp"
#include "esp_err.h"
#include "esp_task.h"
#define SET_BITS(_m, _s, _v)  ((_v) ? (_m)|((_s)) : (_m)&~((_s)))
namespace hardware{

namespace{
    void register_read(i2c_port_t i2c_num, uint8_t reg_addr, uint8_t *data, size_t len)
    {
        ESP_ERROR_CHECK(i2c_master_write_read_device(i2c_num, PCA9557_ADDRESS, &reg_addr, 1, data, len, pdMS_TO_TICKS(1000)));
    }

    void register_write(i2c_port_t i2c_num, uint8_t reg_addr, uint8_t data)
    {
        int ret;
        uint8_t write_buf[2] = {reg_addr, data};
        ESP_ERROR_CHECK(i2c_master_write_to_device(i2c_num, PCA9557_ADDRESS, write_buf, sizeof(write_buf), pdMS_TO_TICKS(1000)));
    }
}
    PCA9557::PCA9557(/* args */)
    {
        register_write(I2C_NUM_0, PCA9557_OUTPUT_PORT, 0x05);
        register_write(I2C_NUM_0, PCA9557_CONFIGURATION_PORT, 0xf8);
    }

    void PCA9557::set_state(uint8_t gpio_bit, uint8_t level){
        uint8_t data;
        register_read(I2C_NUM_0, PCA9557_OUTPUT_PORT, &data, 1);
        register_write(I2C_NUM_0, PCA9557_OUTPUT_PORT, SET_BITS(data, gpio_bit, level));
    }

    PCA9557::~PCA9557()
    {
    }

}