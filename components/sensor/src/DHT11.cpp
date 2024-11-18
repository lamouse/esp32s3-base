#include "DHT11.hpp"
#include "esp_err.h"
#include "esp_log.h"
#include "driver/gpio.h"
#include "esp32s3/rom/ets_sys.h"
#define DHT11_TAG "DHT11"
namespace
{
    // 接收完成回调函数
    static bool IRAM_ATTR example_rmt_rx_done_callback(rmt_channel_handle_t channel, const rmt_rx_done_event_data_t *edata, void *user_data)
    {
        BaseType_t high_task_wakeup = pdFALSE;
        QueueHandle_t rx_receive_queue = (QueueHandle_t)user_data;
        // send the received RMT symbols to the parser task
        xQueueSendFromISR(rx_receive_queue, edata, &high_task_wakeup);
        return high_task_wakeup == pdTRUE;
    }

    int parse_items(rmt_symbol_word_t *item, int item_num, int *humidity, int *temp_x10)
    {
        int i = 0;
        unsigned int rh = 0, temp = 0, checksum = 0;
        if (item_num < 41)
        { // 检查是否有足够的脉冲数
            // ESP_LOGI(TAG, "item_num < 41  %d",item_num);
            return 0;
        }
        if (item_num > 41)
            item++; // 跳过开始信号脉冲

        for (i = 0; i < 16; i++, item++) // 提取湿度数据
        {
            uint16_t duration = 0;
            if (item->level0)
                duration = item->duration0;
            else
                duration = item->duration1;
            rh = (rh << 1) + (duration < 35 ? 0 : 1);
        }

        for (i = 0; i < 16; i++, item++) // 提取温度数据
        {
            uint16_t duration = 0;
            if (item->level0)
                duration = item->duration0;
            else
                duration = item->duration1;
            temp = (temp << 1) + (duration < 35 ? 0 : 1);
        }

        for (i = 0; i < 8; i++, item++)
        { // 提取校验数据

            uint16_t duration = 0;
            if (item->level0)
                duration = item->duration0;
            else
                duration = item->duration1;
            checksum = (checksum << 1) + (duration < 35 ? 0 : 1);
        }
        // 检查校验
        if ((((temp >> 8) + temp + (rh >> 8) + rh) & 0xFF) != checksum)
        {
            ESP_LOGI(DHT11_TAG, "Checksum failure %4X %4X %2X\n", temp, rh, checksum);
            return 0;
        }
        // 返回数据

        rh = rh >> 8;
        temp = (temp >> 8) * 10 + (temp & 0xFF);

        // 判断数据合法性
        if (rh <= 100)
            *humidity = rh;
        if (temp <= 600)
            *temp_x10 = temp;
        return 1;
    }
}

namespace sensor
{
    DHT11::DHT11(gpio_num_t gpio):_gpio(gpio)
    {

        rmt_rx_channel_config_t rx_chan_config = {
            .gpio_num = gpio,
            .clk_src = RMT_CLK_SRC_APB,
            .resolution_hz = 1000 * 1000,
            .mem_block_symbols = 64,
            .flags = {.invert_in = false, .with_dma = false}};
        ESP_ERROR_CHECK(rmt_new_rx_channel(&rx_chan_config, &(this->_chan_handle)));
        _rx_receive = xQueueCreate(20, sizeof(rmt_rx_done_event_data_t));
        assert(_rx_receive);

        // 注册接收完成回调函数
        ESP_LOGI(DHT11_TAG, "register RX done callback");
        rmt_rx_event_callbacks_t cbs = {
            .on_recv_done = example_rmt_rx_done_callback,
        };
        ESP_ERROR_CHECK(rmt_rx_register_event_callbacks(_chan_handle, &cbs, _rx_receive));

        // 使能RMT接收通道
        ESP_ERROR_CHECK(rmt_enable(_chan_handle));
    }

    DHT11::result DHT11::get()
    {
        // 发送20ms开始信号脉冲启动DHT11单总线
        gpio_set_direction(_gpio, GPIO_MODE_OUTPUT);
        gpio_set_level(_gpio, 1);
        ets_delay_us(1000);
        gpio_set_level(_gpio, 0);
        ets_delay_us(20000);
        // 拉高20us
        gpio_set_level(_gpio, 1);
        ets_delay_us(20);
        // 信号线设置为输入准备接收数据
        gpio_set_direction(_gpio, GPIO_MODE_INPUT);
        gpio_set_pull_mode(_gpio, GPIO_PULLUP_ONLY);

        // 启动RMT接收器以获取数据
        rmt_receive_config_t receive_config = {
            .signal_range_min_ns = 100,         // 最小脉冲宽度(0.1us),信号长度小于这个值，视为干扰
            .signal_range_max_ns = 1000 * 1000, // 最大脉冲宽度(1000us)，信号长度大于这个值，视为结束信号
        };

        static rmt_symbol_word_t raw_symbols[128]; // 接收缓存
        static rmt_rx_done_event_data_t rx_data;   // 实际接收到的数据
        ESP_ERROR_CHECK(rmt_receive(_chan_handle, raw_symbols, sizeof(raw_symbols), &receive_config));
        int humidity, temp_x10;
        // wait for RX done signal
        if (xQueueReceive(_rx_receive, &rx_data, pdMS_TO_TICKS(1000)) == pdTRUE)
        {
            // parse the receive symbols and print the result
            parse_items(rx_data.received_symbols, rx_data.num_symbols, &humidity, &temp_x10);
        }
        return {.temperature = temp_x10, .humidity = humidity};
    }

    DHT11::~DHT11()
    {
    }
}
