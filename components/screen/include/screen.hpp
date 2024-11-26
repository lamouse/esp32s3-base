#ifndef SCREEN_HPP
#define SCREEN_HPP

#include "lvgl.h"
#include "driver/i2c.h"
#include "driver//gpio.h"
#include "esp_lcd_types.h"

#define LCD_LEDC_CH (LEDC_CHANNEL_0)
/* LCD display definition */
#define LCD_H_RES              (320)
#define LCD_V_RES              (240)
#define LCD_DRAW_BUFF_HIGHT     (20)
#define LCD_DRAW_BUFF_SIZE     (LCD_H_RES * LCD_V_RES)

/* LCD color formats */
#define ESP_LCD_COLOR_FORMAT_RGB565    (1)
#define ESP_LCD_COLOR_FORMAT_RGB888    (2)

/* LCD display color bits */
#define LCD_BITS_PER_PIXEL      (16)
/* LCD display color space */
#define LCD_COLOR_SPACE         (LCD_RGB_ENDIAN_RGB)



#define LCD_CMD_BITS         (8)
#define LCD_PARAM_BITS       (8)

#define LCD_PIXEL_CLOCK_HZ     (80 * 1000 * 1000)

/* Display */
#define LCD_SPI_NUM         (SPI3_HOST)
#define LCD_SPI_MOSI      (GPIO_NUM_40)
#define LCD_SPI_CLK       (GPIO_NUM_41)
#define LCD_SPI_CS        (GPIO_NUM_NC)
#define LCD_DC            (GPIO_NUM_39)
#define LCD_RST           (GPIO_NUM_NC)
#define LCD_BACKLIGHT     (GPIO_NUM_42)
namespace display
{
    struct display_config_t
    {
        int max_transfer_sz; /*!< Maximum transfer size, in bytes. */
    };
    class screen
    {
    private:
        lv_disp_t *bsp_display_lcd_init();
    public:
        screen();
        void lvgl_start(void);
        void lcd_set_color(uint16_t color);
        void lcd_draw_pictrue(int x_start, int y_start, int x_end, int y_end, const unsigned char *gImage);
        void draw_bitmap(int x_start, int y_start, int x_end, int y_end, const void *color_data);
        void set_brightness(int brightness);
        ~screen();

    };

}

#endif