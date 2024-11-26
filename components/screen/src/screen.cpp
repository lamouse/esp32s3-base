#define LV_USE_PRIVATE_API 1
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_err.h"
#include "esp_check.h"
#include "esp_log.h"
#include "esp_timer.h"
#include "screen.hpp"
#include "driver/ledc.h"
#include "esp_lcd_panel_io.h"
#include "esp_lcd_panel_vendor.h"
#include "esp_lcd_panel_ops.h"
#include "PCA9557.hpp"
#include "esp_lvgl_port.h"
#include "esp_lcd_touch_ft5x06.h"
#include "i2c.hpp"
#include <mutex>


lv_disp_drv_t disp_drv;
#define LCD_WIDTH 320
#define LCD_HEIGHT 240
static const char *TAG = "screen";
namespace display
{
	namespace
	{
		esp_lcd_panel_io_handle_t _io_handle = nullptr;
		esp_lcd_panel_handle_t _panel_handle = nullptr;
		esp_lcd_touch_handle_t _tp;   // 触摸屏句柄
		lv_disp_t *disp;      // 指向液晶屏
		lv_indev_t *disp_indev = NULL; // 指向触摸屏
		std::once_flag init_flag;
		esp_err_t bsp_display_brightness_set(int brightness_percent)
		{
			if (brightness_percent > 100)
			{
				brightness_percent = 100;
			}
			else if (brightness_percent < 0)
			{
				brightness_percent = 0;
			}

			ESP_LOGI(TAG, "Setting LCD backlight: %d%%", brightness_percent);
			// LEDC resolution set to 10bits, thus: 100% = 1023
			uint32_t duty_cycle = (1023 * brightness_percent) / 100;
			ESP_ERROR_CHECK(ledc_set_duty(LEDC_LOW_SPEED_MODE, LCD_LEDC_CH, duty_cycle));
			ESP_ERROR_CHECK(ledc_update_duty(LEDC_LOW_SPEED_MODE, LCD_LEDC_CH));

			return ESP_OK;
		}
		esp_err_t bsp_display_brightness_init(void)
		{
			// Setup LEDC peripheral for PWM backlight control
			const ledc_channel_config_t LCD_backlight_channel = {
				.gpio_num = LCD_BACKLIGHT,
				.speed_mode = LEDC_LOW_SPEED_MODE,
				.channel = LCD_LEDC_CH,
				.intr_type = LEDC_INTR_DISABLE,
				.timer_sel = LEDC_TIMER_1,
				.duty = 0,
				.hpoint = 0,
				.flags = {.output_invert = true}};
			const ledc_timer_config_t LCD_backlight_timer = {
				.speed_mode = LEDC_LOW_SPEED_MODE,
				.duty_resolution = LEDC_TIMER_10_BIT,
				.timer_num = LEDC_TIMER_1,
				.freq_hz = 5000,
				.clk_cfg = LEDC_AUTO_CLK};

			ESP_ERROR_CHECK(ledc_timer_config(&LCD_backlight_timer));
			ESP_ERROR_CHECK(ledc_channel_config(&LCD_backlight_channel));

			return ESP_OK;
		}

		esp_err_t bsp_display_new()
		{
			esp_err_t ret = ESP_OK;

			ESP_RETURN_ON_ERROR(bsp_display_brightness_init(), TAG, "Brightness init failed");

			ESP_LOGD(TAG, "Initialize SPI bus");
			const spi_bus_config_t buscfg = {
				.mosi_io_num = LCD_SPI_MOSI,
				.miso_io_num = GPIO_NUM_NC,
				.sclk_io_num = LCD_SPI_CLK,
				.quadwp_io_num = GPIO_NUM_NC,
				.quadhd_io_num = GPIO_NUM_NC,
				//.max_transfer_sz = LCD_DRAW_BUFF_SIZE * sizeof(uint16_t)
			};
			ESP_RETURN_ON_ERROR(spi_bus_initialize(LCD_SPI_NUM, &buscfg, SPI_DMA_CH_AUTO), TAG, "SPI init failed");

			ESP_LOGD(TAG, "Install panel IO");
			const esp_lcd_panel_io_spi_config_t io_config = {
				.cs_gpio_num = LCD_SPI_CS,
				.dc_gpio_num = LCD_DC,
				.spi_mode = 2,
				.pclk_hz = LCD_PIXEL_CLOCK_HZ,
				.trans_queue_depth = 10,
				.lcd_cmd_bits = LCD_CMD_BITS,
				.lcd_param_bits = LCD_PARAM_BITS,
			};
			ESP_ERROR_CHECK(esp_lcd_new_panel_io_spi((esp_lcd_spi_bus_handle_t)LCD_SPI_NUM, &io_config, &_io_handle));

			ESP_LOGD(TAG, "Install LCD driver");
			const esp_lcd_panel_dev_config_t panel_config = {
				.reset_gpio_num = LCD_RST,
				.rgb_ele_order = LCD_RGB_ELEMENT_ORDER_RGB,
				.bits_per_pixel = LCD_BITS_PER_PIXEL,
			};
			ESP_ERROR_CHECK(esp_lcd_new_panel_st7789(_io_handle, &panel_config, &_panel_handle));

			esp_lcd_panel_reset(_panel_handle);
			hardware::PCA9557 pca;
			pca.set_state(LCD_CS_GPIO, 0);
			esp_lcd_panel_init(_panel_handle);
			esp_lcd_panel_invert_color(_panel_handle, true);
			esp_lcd_panel_swap_xy(_panel_handle, true);		  // 显示翻转
			esp_lcd_panel_mirror(_panel_handle, true, false); // 镜像
			return ret;
		}

		// 触摸屏初始化
		esp_err_t bsp_touch_new(esp_lcd_touch_handle_t *ret_touch)
		{
			/* Initialize touch */
			esp_lcd_touch_config_t tp_cfg = {
				.x_max = LCD_V_RES,
				.y_max = LCD_H_RES,
				.rst_gpio_num = GPIO_NUM_NC, // Shared with LCD reset
				.int_gpio_num = GPIO_NUM_NC,
				.levels = {
					.reset = 0,
					.interrupt = 0,
				},
				.flags = {
					.swap_xy = 1,
					.mirror_x = 1,
					.mirror_y = 0,
				},
			};
			esp_lcd_panel_io_handle_t tp_io_handle = NULL;
			esp_lcd_panel_io_i2c_config_t tp_io_config = ESP_LCD_TOUCH_IO_I2C_FT5x06_CONFIG();

			esp_lcd_new_panel_io_i2c((esp_lcd_i2c_bus_handle_t)I2C_MASTER_PORT, &tp_io_config, &tp_io_handle);
			ESP_ERROR_CHECK(esp_lcd_touch_new_i2c_ft5x06(tp_io_handle, &tp_cfg, ret_touch));

			return ESP_OK;
		}

		lv_indev_t *bsp_display_indev_init(lv_disp_t *disp)
		{
			/* 初始化触摸屏 */
			ESP_ERROR_CHECK(bsp_touch_new(&_tp));
			assert(_tp);

			/* 添加LVGL接口 */
			const lvgl_port_touch_cfg_t touch_cfg = {
				.disp = disp,
				.handle = _tp,
			};

			return lvgl_port_add_touch(&touch_cfg);
		}
	}



screen::screen(/* args */)
{
	std::call_once(init_flag, [&]()
				   {
			lvgl_start();
		// auto ret = bsp_display_new(); // 液晶屏驱动初始化
		// lcd_set_color(0xFFFF);									 // 设置整屏背景黑色
		// ret = esp_lcd_panel_disp_on_off(_panel_handle, true);	 // 打开液晶屏显示
		// ret = bsp_display_brightness_set(100);
		}); // 打开背光显示
}

void screen::lvgl_start(void)
{
    /* 初始化LVGL */
    lvgl_port_cfg_t lvgl_cfg = ESP_LVGL_PORT_INIT_CONFIG();
    lvgl_port_init(&lvgl_cfg);

    /* 初始化液晶屏 并添加LVGL接口 */
    disp = bsp_display_lcd_init();

    /* 初始化触摸屏 并添加LVGL接口 */
    disp_indev = bsp_display_indev_init(disp);

    /* 打开液晶屏背光 */
    bsp_display_brightness_set(100);// 打开背光显示
}

void screen::set_brightness(int brightness)
{
	bsp_display_brightness_set(brightness);
}

// 液晶屏初始化+添加LVGL接口
lv_disp_t* screen::bsp_display_lcd_init(void)
{
	/* 初始化液晶屏 */
	bsp_display_new();								// 液晶屏驱动初始化
	lcd_set_color(0xffff);							// 设置整屏背景白色
	esp_lcd_panel_disp_on_off(_panel_handle, true); // 打开液晶屏显示

	/* 液晶屏添加LVGL接口 */
	ESP_LOGD(TAG, "Add LCD screen");
	const lvgl_port_display_cfg_t disp_cfg = {
		.io_handle = _io_handle,
		.panel_handle = _panel_handle,
		.buffer_size = LCD_H_RES * LCD_DRAW_BUFF_HIGHT, // LVGL缓存大小
		.double_buffer = true,							// 是否开启双缓存
		.hres = LCD_H_RES,								// 液晶屏的宽
		.vres = LCD_V_RES,								// 液晶屏的高
		.monochrome = false,							// 是否单色显示器
		/* Rotation的值必须和液晶屏初始化里面设置的 翻转 和 镜像 一样 */
		.rotation = {
			.swap_xy = true,   // 是否翻转
			.mirror_x = true,  // x方向是否镜像
			.mirror_y = false, // y方向是否镜像
		},
		.flags = {
			.buff_dma = false,	 // 是否使用DMA 注意：dma与spiram不能同时为true
			.buff_spiram = true, // 是否使用PSRAM 注意：dma与spiram不能同时为true
		}};

	return lvgl_port_add_disp(&disp_cfg);
}

screen::~screen()
{
	if (_panel_handle)
	{
		esp_lcd_panel_del(_panel_handle);
	}
	if (_io_handle)
	{
		esp_lcd_panel_io_del(_io_handle);
	}
	spi_bus_free(LCD_SPI_NUM);
}

// 显示图片
void screen::lcd_draw_pictrue(int x_start, int y_start, int x_end, int y_end, const unsigned char *gImage)
{
	// 分配内存 分配了需要的字节大小 且指定在外部SPIRAM中分配
	size_t pixels_byte_size = (x_end - x_start) * (y_end - y_start) * 2;
	uint16_t *pixels = (uint16_t *)heap_caps_malloc(pixels_byte_size, MALLOC_CAP_8BIT | MALLOC_CAP_SPIRAM);
	if (NULL == pixels)
	{
		ESP_LOGE(TAG, "Memory for bitmap is not enough");
		return;
	}
	memcpy(pixels, gImage, pixels_byte_size);													  // 把图片数据拷贝到内存
	esp_lcd_panel_draw_bitmap(_panel_handle, x_start, y_start, x_end, y_end, (uint16_t *)pixels); // 显示整张图片数据
	heap_caps_free(pixels);																		  // 释放内存
}

// 设置液晶屏颜色
void screen::lcd_set_color(uint16_t color)
{
	// 分配内存 这里分配了液晶屏一行数据需要的大小
	uint16_t *buffer = (uint16_t *)heap_caps_malloc(LCD_H_RES * sizeof(uint16_t), MALLOC_CAP_8BIT | MALLOC_CAP_SPIRAM);

	if (NULL == buffer)
	{
		ESP_LOGE(TAG, "Memory for bitmap is not enough");
	}
	else
	{
		for (size_t i = 0; i < LCD_H_RES; i++) // 给缓存中放入颜色数据
		{
			buffer[i] = color;
		}
		for (int y = 0; y < 240; y++) // 显示整屏颜色
		{
			esp_lcd_panel_draw_bitmap(_panel_handle, 0, y, 320, y + 1, buffer);
		}
		free(buffer); // 释放内存
	}
}
void screen::draw_bitmap(int x_start, int y_start, int x_end, int y_end, const void *color_data)
{
	esp_lcd_panel_draw_bitmap(_panel_handle, x_start, y_start, x_end, y_end, color_data);
}
}