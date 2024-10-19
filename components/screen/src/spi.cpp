#include <spi.hpp>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_timer.h"
#include "esp_lcd_panel_io.h"
#include "esp_lcd_panel_vendor.h"
#include "esp_lcd_panel_ops.h"
#include "esp_lcd_panel_commands.h"
#include <driver/spi_master.h>
#include <driver/gpio.h>
#include <esp_lcd_panel_io.h>
#define HSPI_HOST SPI2_HOST
#define LCD_SPI_HOST    SPI2_HOST


namespace driver {

namespace{
    lcd_flush_callback_t lcd_flush_callback;
}
SPI::SPI(uint16_t width, lcd_flush_callback_t callback) {
    lcd_flush_callback = callback;
    spi_bus_config_t buscfg = {
        //TODO: Change to correct pins
        .mosi_io_num = 23,
        .miso_io_num = 19,
        .sclk_io_num = 18,
        .quadwp_io_num = -1,
        .quadhd_io_num = -1,
        .max_transfer_sz =  width * 40 * sizeof(uint8_t),
        .flags = SPICOMMON_BUSFLAG_MASTER,
    };
    ESP_ERROR_CHECK(spi_bus_initialize(HSPI_HOST, &buscfg, SPI_DMA_CH_AUTO));
    
    gpio_config_t bl_gpio_config = {
        //TODO: Change to correct pins
        .pin_bit_mask = 1 << GPIO_NUM_17,
        .mode = GPIO_MODE_OUTPUT,
        .pull_up_en = GPIO_PULLUP_DISABLE,
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .intr_type = GPIO_INTR_DISABLE,
    };

    ESP_ERROR_CHECK(gpio_config(&bl_gpio_config));

    gpio_config_t rst_gpio_config = {
        .pin_bit_mask = 1 << GPIO_NUM_4,
        .mode = GPIO_MODE_OUTPUT,
        .pull_up_en = GPIO_PULLUP_DISABLE,
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .intr_type = GPIO_INTR_DISABLE,
    };
    ESP_ERROR_CHECK(gpio_config(&rst_gpio_config));

    esp_lcd_panel_io_spi_config_t spi_lcd_io_config = {
        .cs_gpio_num = GPIO_NUM_15,
        .dc_gpio_num = GPIO_NUM_12,
        .spi_mode = 0,
        .pclk_hz = 10 * 1000 * 1000,
        .trans_queue_depth = 16,
        .on_color_trans_done = [](esp_lcd_panel_io_handle_t panel_io, esp_lcd_panel_io_event_data_t *edata, void *user_ctx)->bool {
            lcd_flush_callback(user_ctx);
            return false;
        },
        .user_ctx = this, //TODO: Change to correct pins
        .lcd_cmd_bits = 8,
        .lcd_param_bits = 8,
        .flags = {.sio_mode = 0},
    };

    ESP_ERROR_CHECK(esp_lcd_new_panel_io_spi((esp_lcd_spi_bus_handle_t)LCD_SPI_HOST, &spi_lcd_io_config, &lcd_io_handle));

    gpio_set_level(GPIO_NUM_4, 0);
    vTaskDelay(pdMS_TO_TICKS(20));
    gpio_set_level(GPIO_NUM_4, 1);
    vTaskDelay(pdMS_TO_TICKS(20));

    esp_lcd_panel_io_tx_param(lcd_io_handle, LCD_CMD_SWRESET, nullptr, 0);
    vTaskDelay(pdMS_TO_TICKS(150));
    esp_lcd_panel_io_tx_param(lcd_io_handle, LCD_CMD_SLPOUT, nullptr, 0);
    vTaskDelay(pdMS_TO_TICKS(200));
    esp_lcd_panel_io_tx_param(lcd_io_handle,LCD_CMD_COLMOD,(uint8_t[]) {0x55,}, 1);  //选择RGB数据格式，0x55:RGB565,0x66:RGB666
    esp_lcd_panel_io_tx_param(lcd_io_handle, 0xb0, (uint8_t[]) {0x00, 0xF0},2);

    esp_lcd_panel_io_tx_param(lcd_io_handle,LCD_CMD_INVON,NULL,0);     //颜色翻转
    esp_lcd_panel_io_tx_param(lcd_io_handle,LCD_CMD_NORON,NULL,0);     //普通显示模式

    uint8_t spin_type = 0;
    switch(0)
    {
        case 0:
            spin_type = 0x00;   //不旋转
            break;
        case 1:
            spin_type = 0x60;   //顺时针90
            break;
        case 2:
            spin_type = 0xC0;   //180
            break;
        case 3:
            spin_type = 0xA0;   //顺时针270,（逆时针90）
            break;
        default:break;
    }
    esp_lcd_panel_io_tx_param(lcd_io_handle,LCD_CMD_MADCTL,(uint8_t[]) {spin_type,}, 1);   //屏旋转方向
    vTaskDelay(pdMS_TO_TICKS(150));
    esp_lcd_panel_io_tx_param(lcd_io_handle,LCD_CMD_DISPON,NULL,0);    //打开显示
    vTaskDelay(pdMS_TO_TICKS(300));
}

SPI::~SPI() {}

void SPI::write(int x1, int y1, int x2, int y2, void* data) {
       if(x2 <= x1 || y2 <= y1)
    {
        lcd_flush_callback(nullptr);
        return;
    }
    esp_lcd_panel_io_tx_param(lcd_io_handle, LCD_CMD_CASET, (uint8_t[]) {
        (x1 >> 8) & 0xFF,
        x1 & 0xFF,
        ((x2 - 1) >> 8) & 0xFF,
        (x2 - 1) & 0xFF,
    }, 4);
    esp_lcd_panel_io_tx_param(lcd_io_handle, LCD_CMD_RASET, (uint8_t[]) {
        (y1 >> 8) & 0xFF,
        y1 & 0xFF,
        ((y2 - 1) >> 8) & 0xFF,
        (y2 - 1) & 0xFF,
    }, 4);
    // transfer frame buffer
    size_t len = (x2 - x1) * (y2 - y1) * 2;
    esp_lcd_panel_io_tx_color(lcd_io_handle, LCD_CMD_RAMWR, data, len);
}
void SPI::read(uint8_t *data, size_t len) {}
void SPI::transfer(uint8_t *data, size_t len) {}
void SPI::set_speed(uint32_t speed) {}
void SPI::set_mode(uint8_t mode) {}
} // namespace driver