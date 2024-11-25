#include "i2c.hpp"
#include <mutex>

namespace hardware
{
    namespace
    {
        std::once_flag init_flag;
    }
    i2c_master::i2c_master()
    {
        std::call_once(init_flag, []()
                       {
                   i2c_config_t i2c_conf{
            .mode = I2C_MODE_MASTER,
            .sda_io_num = I2C_MASTER_SDA,
            .scl_io_num = I2C_MASTER_SCL,
            .sda_pullup_en = GPIO_PULLUP_ENABLE,
            .scl_pullup_en = GPIO_PULLUP_ENABLE,
            .master = {
                .clk_speed = I2C_MASTER_SCL_SPEED}};
        ESP_ERROR_CHECK(i2c_param_config(I2C_MASTER_PORT, &i2c_conf));
        ESP_ERROR_CHECK(i2c_driver_install(I2C_MASTER_PORT, i2c_conf.mode, 0, 0, 0)); });
    }

    void i2c_master::register_read(uint8_t device_address, uint8_t reg_addr, uint8_t *data, size_t len){
        ESP_ERROR_CHECK(i2c_master_write_read_device(I2C_MASTER_PORT, device_address, &reg_addr, 1, data, len, pdMS_TO_TICKS(1000)));
    }

    void i2c_master::register_write(uint8_t device_address, uint8_t reg_addr, uint8_t data)
    {
        int ret;
        uint8_t write_buf[2] = {reg_addr, data};
        ESP_ERROR_CHECK(i2c_master_write_to_device(I2C_MASTER_PORT, device_address, write_buf, sizeof(write_buf), pdMS_TO_TICKS(1000)));
    }

    i2c_master::~i2c_master()
    {
        //i2c_driver_delete(I2C_MASTER_PORT);
    }

}