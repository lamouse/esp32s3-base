#ifndef KEY_HPP
#define KEY_HPP
#include "driver/gpio.h"
#include "freertos/FreeRTOS.h"
namespace hardware{
    enum class key_stat{
        put_down,
        put_up,
        no_action
    };
    class  key
    {
    private:
        gpio_num_t _gpio;
        bool _put_down;
        bool _put_up;
    public:
        key(gpio_num_t gpio);
        key_stat get_state();
        friend void gpio_isr_handle(void *arg);
        ~ key();
    };
}
#endif