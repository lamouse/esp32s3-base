#ifndef DHT11_HPP
#define DHT11_HPP
#include "freertos/FreeRTOS.h"
#include "driver/rmt_rx.h"

namespace sensor{

    class DHT11
    {
    private:
        rmt_channel_handle_t _chan_handle;
        QueueHandle_t _rx_receive;
        gpio_num_t _gpio;
    public:
        struct result
        {
            int temperature;
            int humidity;
        };
        DHT11(gpio_num_t gpio);
        result get();
        ~DHT11();
    };




}
#endif