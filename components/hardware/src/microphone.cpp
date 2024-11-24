#include "microphone.hpp"
#include <string.h>
#include "sdkconfig.h"
#include "esp_check.h"
#include "esp_vfs_fat.h"
#include "esp_log.h"
#include "driver/i2c.h"
#include "es7210.h"
#include "format_wav.h"
#include "driver/gpio.h"
#include "sd_card.hpp"
#include <fstream>
#include <filesystem>
#include <chrono>

#define EXAMPLE_I2C_NUM I2C_NUM_0
#define EXAMPLE_I2C_SDA_IO (1)
#define EXAMPLE_I2C_SCL_IO (2)

/* I2S port and GPIOs */
#define EXAMPLE_I2S_NUM (0)
#define EXAMPLE_I2S_MCK_IO GPIO_NUM_38
#define EXAMPLE_I2S_BCK_IO GPIO_NUM_14
#define EXAMPLE_I2S_WS_IO GPIO_NUM_13
#define EXAMPLE_I2S_DI_IO GPIO_NUM_12

/* I2S configurations */
#define EXAMPLE_I2S_TDM_FORMAT (ES7210_I2S_FMT_I2S)
#define EXAMPLE_I2S_CHAN_NUM (2)
#define EXAMPLE_I2S_SAMPLE_RATE (48000)
#define EXAMPLE_I2S_MCLK_MULTIPLE (I2S_MCLK_MULTIPLE_256)
#define EXAMPLE_I2S_SAMPLE_BITS (I2S_DATA_BIT_WIDTH_16BIT)
#define EXAMPLE_I2S_TDM_SLOT_MASK (I2S_TDM_SLOT0 | I2S_TDM_SLOT1)

/* ES7210 configurations */
#define EXAMPLE_ES7210_I2C_ADDR (0x41)
#define EXAMPLE_ES7210_I2C_CLK (100000)
#define EXAMPLE_ES7210_MIC_GAIN (ES7210_MIC_GAIN_30DB)
#define EXAMPLE_ES7210_MIC_BIAS (ES7210_MIC_BIAS_2V87)
#define EXAMPLE_ES7210_ADC_VOLUME (0)
#define RECORD_FILE_DIR   "/microphone"

#define RECORD_TIME_SEC 10
namespace
{
    const char *MICROPHONE_TAG = "microphone";
    std::string record_path;
}
namespace hardware
{
    void create_record_path(){
        auto root = sd_card::get_mount_point();
        record_path = root + "/";
        //if(!fs::exists(record_path)){
        //     if(!fs::create_directories(record_path)){
        //         ESP_LOGE(MICROPHONE_TAG, "create microphone record dir %s err", record_path.c_str());
        //     }
        //}
    }

    std::string get_current_time(){
        auto now = std::chrono::system_clock::now();
        std::time_t now_time = std::chrono::system_clock::to_time_t(now);
        // 使用字符串流格式化时间
        std::stringstream ss; ss << std::put_time(std::localtime(&now_time), "%Y-%m-%d %H:%M:%S"); return ss.str();
    }

    void es7210_codec_init(void)
    {
        ESP_LOGI(MICROPHONE_TAG, "Init I2C used to configure ES7210");
        i2c_config_t i2c_conf = {
            .mode = I2C_MODE_MASTER,
            .sda_io_num = EXAMPLE_I2C_SDA_IO,
            .scl_io_num = EXAMPLE_I2C_SCL_IO,
            .sda_pullup_en = GPIO_PULLUP_ENABLE,
            .scl_pullup_en = GPIO_PULLUP_ENABLE,
            .master = {.clk_speed = EXAMPLE_ES7210_I2C_CLK}};
        ESP_ERROR_CHECK(i2c_param_config(EXAMPLE_I2C_NUM, &i2c_conf));
        ESP_ERROR_CHECK(i2c_driver_install(EXAMPLE_I2C_NUM, i2c_conf.mode, 0, 0, 0));
        /* Create ES7210 device handle */
        es7210_dev_handle_t es7210_handle = NULL;
        es7210_i2c_config_t es7210_i2c_conf = {
            .i2c_port = EXAMPLE_I2C_NUM,
            .i2c_addr = EXAMPLE_ES7210_I2C_ADDR};
        ESP_ERROR_CHECK(es7210_new_codec(&es7210_i2c_conf, &es7210_handle));

        ESP_LOGI(MICROPHONE_TAG, "Configure ES7210 codec parameters");
        es7210_codec_config_t codec_conf = {
            .sample_rate_hz = EXAMPLE_I2S_SAMPLE_RATE,
            .mclk_ratio = EXAMPLE_I2S_MCLK_MULTIPLE,
            .i2s_format = EXAMPLE_I2S_TDM_FORMAT,
            .bit_width = (es7210_i2s_bits_t)EXAMPLE_I2S_SAMPLE_BITS,
            .mic_bias = EXAMPLE_ES7210_MIC_BIAS,
            .mic_gain = EXAMPLE_ES7210_MIC_GAIN,
            .flags = {.tdm_enable = true}};
        ESP_ERROR_CHECK(es7210_config_codec(es7210_handle, &codec_conf));
        ESP_ERROR_CHECK(es7210_config_volume(es7210_handle, EXAMPLE_ES7210_ADC_VOLUME));
    }

    esp_err_t record_wav(i2s_chan_handle_t i2s_rx_chan)
    {
        create_record_path();
        ESP_RETURN_ON_FALSE(i2s_rx_chan, ESP_FAIL, MICROPHONE_TAG, "invalid i2s channel handle pointer");
        esp_err_t ret = ESP_OK;

        uint32_t byte_rate = EXAMPLE_I2S_SAMPLE_RATE * EXAMPLE_I2S_CHAN_NUM * EXAMPLE_I2S_SAMPLE_BITS / 8;
        uint32_t wav_size = byte_rate * RECORD_TIME_SEC;

        const wav_header_t wav_header =
            WAV_HEADER_PCM_DEFAULT(wav_size, EXAMPLE_I2S_SAMPLE_BITS, EXAMPLE_I2S_SAMPLE_RATE, EXAMPLE_I2S_CHAN_NUM);
        std::ofstream ofs;
        ofs.open(sd_card::get_mount_point() + "/123.WAV");
        if(!ofs.is_open()){
            ESP_LOGI(MICROPHONE_TAG, "Recording file error");
        }
        ofs.write(reinterpret_cast<const char*> (&wav_header), sizeof(wav_header_t));
        /* Start recording */
        size_t wav_written = 0;
        static int16_t i2s_readraw_buff[4096];
        ESP_GOTO_ON_ERROR(i2s_channel_enable(i2s_rx_chan), err, MICROPHONE_TAG, "error while starting i2s rx channel");
        while (wav_written < wav_size)
        {
            if (wav_written % byte_rate < sizeof(i2s_readraw_buff))
            {
                ESP_LOGI(MICROPHONE_TAG, "Recording: %" PRIu32 "/%ds", wav_written / byte_rate + 1, RECORD_TIME_SEC);
            }
            size_t bytes_read = 0;
            /* Read RAW samples from ES7210 */
            ESP_GOTO_ON_ERROR(i2s_channel_read(i2s_rx_chan, i2s_readraw_buff, sizeof(i2s_readraw_buff), &bytes_read,
                                               pdMS_TO_TICKS(1000)),
                              err, MICROPHONE_TAG, "error while reading samples from i2s");
            /* Write the samples to the WAV file */
            ofs.write(reinterpret_cast<const char*>(i2s_readraw_buff), bytes_read);
            wav_written += bytes_read;
        }
        ofs.flush();
    err:
        i2s_channel_disable(i2s_rx_chan);
        ESP_LOGI(MICROPHONE_TAG, "Recording done! Flushing file buffer");

        return ret;
    }
    microphone::microphone(/* args */)
    {
        ESP_LOGI(MICROPHONE_TAG, "Create I2S receive channel");
        i2s_chan_config_t i2s_rx_conf = I2S_CHANNEL_DEFAULT_CONFIG(I2S_NUM_AUTO, I2S_ROLE_MASTER);
        ESP_ERROR_CHECK(i2s_new_channel(&i2s_rx_conf, NULL, &_i2s_rx_chan));

        ESP_LOGI(MICROPHONE_TAG, "Configure I2S receive channel to TDM mode");
        i2s_tdm_config_t i2s_tdm_rx_conf = {

            .clk_cfg = {
                .sample_rate_hz = EXAMPLE_I2S_SAMPLE_RATE,
                .clk_src = I2S_CLK_SRC_DEFAULT,
                .mclk_multiple = EXAMPLE_I2S_MCLK_MULTIPLE},
            .slot_cfg = I2S_TDM_PHILIPS_SLOT_DEFAULT_CONFIG(EXAMPLE_I2S_SAMPLE_BITS, I2S_SLOT_MODE_STEREO, static_cast<i2s_tdm_slot_mask_t>(EXAMPLE_I2S_TDM_SLOT_MASK)),
            .gpio_cfg = {.mclk = EXAMPLE_I2S_MCK_IO, .bclk = EXAMPLE_I2S_BCK_IO, .ws = EXAMPLE_I2S_WS_IO,
                         .dout = GPIO_NUM_NC, // ES7210 only has ADC capability
                         .din = EXAMPLE_I2S_DI_IO
                         },
        };

        ESP_ERROR_CHECK(i2s_channel_init_tdm_mode(_i2s_rx_chan, &i2s_tdm_rx_conf));
        es7210_codec_init();
        record_wav(_i2s_rx_chan);
    }

    microphone::~microphone()
    {
    }
}