#include "audio.hpp"
#include <stdio.h>
#include <string.h>
#include "sdkconfig.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_system.h"
#include "esp_check.h"
#include "es8311.h"
#include "i2c.hpp"

static const char *AUDIO_TAG = "audio";
extern const uint8_t music_pcm_start[] asm("_binary_canon_pcm_start");
extern const uint8_t music_pcm_end[] asm("_binary_canon_pcm_end");
static const char err_reason[][30] = {"input param is invalid",
                                      "operation timeout"};
namespace hardware
{

    audio::audio(/* args */)
    {

        if (i2s_driver_init() != ESP_OK)
        {
            ESP_LOGE(AUDIO_TAG, "i2s driver init failed");
            return;
        }
        i2c_master i2c;

        /* Initialize es8311 codec */
        es8311_handle_t es_handle = es8311_create(I2C_NUM, ES8311_ADDRRES_0);
        if (es_handle == nullptr)
        {
            ESP_LOGE(AUDIO_TAG, "init es8311 error");
        }
        const es8311_clock_config_t es_clk = {
            .mclk_inverted = false,
            .sclk_inverted = false,
            .mclk_from_mclk_pin = true,
            .mclk_frequency = EXAMPLE_MCLK_FREQ_HZ,
            .sample_frequency = EXAMPLE_SAMPLE_RATE};

        ESP_ERROR_CHECK(es8311_init(es_handle, &es_clk, ES8311_RESOLUTION_16, ES8311_RESOLUTION_16));
        ESP_ERROR_CHECK(es8311_sample_frequency_config(es_handle, EXAMPLE_SAMPLE_RATE * EXAMPLE_MCLK_MULTIPLE, EXAMPLE_SAMPLE_RATE));
        ESP_ERROR_CHECK(es8311_voice_volume_set(es_handle, EXAMPLE_VOICE_VOLUME, NULL));
        ESP_ERROR_CHECK(es8311_microphone_config(es_handle, false));

        data_ptr = (uint8_t *)music_pcm_start;

        /* (Optional) Disable TX channel and preload the data before enabling the TX channel,
         * so that the valid data can be transmitted immediately */
        ESP_ERROR_CHECK(i2s_channel_disable(tx_handle));
        ESP_ERROR_CHECK(i2s_channel_preload_data(tx_handle, data_ptr, music_pcm_end - data_ptr, &bytes_write));
        data_ptr += bytes_write; // Move forward the data pointer

        /* Enable the TX channel */
        ESP_ERROR_CHECK(i2s_channel_enable(tx_handle));
    }

    esp_err_t audio::i2s_driver_init(void)
    {
        i2s_chan_config_t chan_cfg = I2S_CHANNEL_DEFAULT_CONFIG(I2S_NUM, I2S_ROLE_MASTER);
        chan_cfg.auto_clear = true; // Auto clear the legacy data in the DMA buffer
        ESP_ERROR_CHECK(i2s_new_channel(&chan_cfg, &tx_handle, nullptr));
        i2s_std_config_t std_cfg = {
            .clk_cfg = I2S_STD_CLK_DEFAULT_CONFIG(EXAMPLE_SAMPLE_RATE),
            .slot_cfg = I2S_STD_PHILIPS_SLOT_DEFAULT_CONFIG(I2S_DATA_BIT_WIDTH_16BIT, I2S_SLOT_MODE_STEREO),
            .gpio_cfg = {
                .mclk = I2S_MCK_IO,
                .bclk = I2S_BCK_IO,
                .ws = I2S_WS_IO,
                .dout = I2S_DO_IO,
                .din = I2S_DI_IO,
                .invert_flags = {
                    .mclk_inv = false,
                    .bclk_inv = false,
                    .ws_inv = false,
                },
            },
        };
        std_cfg.clk_cfg.mclk_multiple = EXAMPLE_MCLK_MULTIPLE;

        ESP_ERROR_CHECK(i2s_channel_init_std_mode(tx_handle, &std_cfg));
        ESP_ERROR_CHECK(i2s_channel_enable(tx_handle));
        return ESP_OK;
    }

    void audio::i2s_music(void *args)
    {
        esp_err_t ret = ESP_OK;


            /* Write music to earphone */
            ret = i2s_channel_write(tx_handle, data_ptr, music_pcm_end - data_ptr, &bytes_write, portMAX_DELAY);
            if (ret != ESP_OK)
            {
                /* Since we set timeout to 'portMAX_DELAY' in 'i2s_channel_write'
                   so you won't reach here unless you set other timeout value,
                   if timeout detected, it means write operation failed. */
                ESP_LOGE(AUDIO_TAG, "[music] i2s write failed, %s", err_reason[ret == ESP_ERR_TIMEOUT]);
                abort();
            }
            if (bytes_write > 0)
            {
                ESP_LOGI(AUDIO_TAG, "[music] i2s music played, %d bytes are written.", bytes_write);
            }
            else
            {
                ESP_LOGE(AUDIO_TAG, "[music] i2s music play failed.");
                abort();
            }
    }

    audio::~audio()
    {
    }
}