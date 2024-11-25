#include "PCA9557.hpp"
#include "esp_err.h"
#include "esp_task.h"
#include "i2c.hpp"
#include <mutex>

#define SET_BITS(_m, _s, _v) ((_v) ? (_m) | ((_s)) : (_m) & ~((_s)))
namespace hardware
{

    namespace
    {
        std::once_flag init_flag;
    }
    PCA9557::PCA9557(/* args */)
    {
        std::call_once(init_flag, []()
                       {
            i2c_master i2c;
            i2c.register_write(PCA9557_ADDRESS ,PCA9557_OUTPUT_PORT, 0x05);
            i2c.register_write(PCA9557_ADDRESS ,PCA9557_CONFIGURATION_PORT, 0xf8); });
    }

    void PCA9557::set_state(uint8_t gpio_bit, uint8_t level)
    {
        i2c_master i2c;
        uint8_t data;
        i2c.register_read(PCA9557_ADDRESS, PCA9557_OUTPUT_PORT, &data, 1);
        i2c.register_write(PCA9557_ADDRESS, PCA9557_OUTPUT_PORT, SET_BITS(data, gpio_bit, level));
    }

    PCA9557::~PCA9557()
    {
    }

}