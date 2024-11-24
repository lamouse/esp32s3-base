#pragma once
#include "driver/i2s_std.h"
#include "driver/gpio.h"
#define I2C_SCL_IO      (GPIO_NUM_2)
#define I2C_SDA_IO      (GPIO_NUM_1)
#define I2S_NUM         (I2S_NUM_0)
#define I2C_NUM         (I2C_NUM_0)
#define EXAMPLE_SAMPLE_RATE     (16000)
#define EXAMPLE_MCLK_MULTIPLE   (I2S_MCLK_MULTIPLE_384) // If not using 24-bit data width, 256 should be enough
#define EXAMPLE_MCLK_FREQ_HZ    (EXAMPLE_SAMPLE_RATE * EXAMPLE_MCLK_MULTIPLE)
#define EXAMPLE_VOICE_VOLUME    60

#define I2S_MCK_IO      (GPIO_NUM_38)
#define I2S_BCK_IO      (GPIO_NUM_14)
#define I2S_WS_IO       (GPIO_NUM_13)
#define I2S_DO_IO       (GPIO_NUM_45)
#define I2S_DI_IO       (GPIO_NUM_NC)

namespace hardware{
    class audio
    {
    private:
        i2s_chan_handle_t tx_handle = nullptr;
        esp_err_t i2s_driver_init(void);
        uint8_t *data_ptr;
        size_t bytes_write = 0;
    public:
        audio(/* args */);
        void i2s_music(void *args);
        ~audio();
    };
}