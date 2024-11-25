#pragma once
#include "driver/i2c.h"
#define I2C_MASTER_PORT I2C_NUM_0
#define I2C_MASTER_SDA GPIO_NUM_1
#define I2C_MASTER_SCL GPIO_NUM_2
#define I2C_MASTER_SCL_SPEED 400 * 1000
/**
 * @brief 使用i2c功能的建议使用这里面的类，由这些类保证i2c只进行一次初始化，避免重复初始化的错误
 *
 */
namespace hardware{
    class i2c_master
    {
    private:
        /* data */
    public:
        i2c_master();
        void register_read(uint8_t device_address, uint8_t reg_addr, uint8_t *data, size_t len);
        void register_write(uint8_t device_address, uint8_t reg_addr, uint8_t data);
        ~i2c_master();
    };

}