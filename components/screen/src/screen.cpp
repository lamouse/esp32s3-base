#define LV_USE_PRIVATE_API 1
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_err.h"
#include "esp_log.h"
#include "esp_timer.h"
#include "lvgl.h"
#include "screen.hpp"
#include "spi.hpp"
lv_disp_drv_t disp_drv;
#define LCD_WIDTH 320
#define LCD_HEIGHT 240
static const char *TAG = "screen";
namespace display
{
  namespace
  {
    void lv_tick_inc_cb(void *data)
    {
      uint32_t tick_inc_period_ms = *((uint32_t *)data);
      lv_tick_inc(tick_inc_period_ms);
    }

    void lv_port_flush_ready(void *param)
    {
      lv_disp_flush_ready(&disp_drv);

      /* portYIELD_FROM_ISR (true) or not (false). */
    }

    esp_err_t lv_port_indev_init(void)
    {
      // static lv_indev_drv_t indev_drv;
      // lv_indev_drv_init(&indev_drv);
      // indev_drv.type = LV_INDEV_TYPE_POINTER;
      // indev_drv.read_cb = indev_read;
      // lv_indev_drv_register(&indev_drv);
      return ESP_OK;
    }

    esp_err_t lv_port_tick_init(void)
    {
      static uint32_t tick_inc_period_ms = 5;
      const esp_timer_create_args_t periodic_timer_args = {
          .callback = lv_tick_inc_cb,
          .arg = &tick_inc_period_ms,
          .dispatch_method = ESP_TIMER_TASK,
          .name = "",
          .skip_unhandled_events = true,
      };

      esp_timer_handle_t periodic_timer;
      ESP_ERROR_CHECK(esp_timer_create(&periodic_timer_args, &periodic_timer));
      ESP_ERROR_CHECK(esp_timer_start_periodic(periodic_timer, tick_inc_period_ms * 1000));

      return ESP_OK;
    }

    // void lcd_init(void)
    // {
    //     st7789_cfg_t st7789_config;
    //     st7789_config.mosi = GPIO_NUM_19;
    //     st7789_config.clk = GPIO_NUM_18;
    //     st7789_config.cs = GPIO_NUM_5;
    //     st7789_config.dc = GPIO_NUM_17;
    //     st7789_config.rst = GPIO_NUM_21;
    //     st7789_config.bl = GPIO_NUM_26;
    //     st7789_config.spi_fre = 40*1000*1000;       //SPI时钟频率
    //     st7789_config.width = LCD_WIDTH;            //屏宽
    //     st7789_config.height = LCD_HEIGHT;          //屏高
    //     st7789_config.spin = 1;                     //顺时针旋转90度
    //     st7789_config.done_cb = lv_port_flush_ready;    //数据写入完成回调函数
    //     st7789_config.cb_param = &disp_drv;         //回调函数参数

    //     st7789_driver_hw_init(&st7789_config);
    // }
  }

screen* screen::_instance = nullptr;
  /**
   * @brief 写入显示数据
   *
   * @param disp_drv  对应的显示器
   * @param area      显示区域
   * @param color_p   显示数据
   */
  void screen::disp_flush(lv_disp_drv_t *disp_drv, const lv_area_t *area, lv_color_t *color_p)
  {
    (void)disp_drv;
    spi.write(area->x1 + 20, area->x2 + 1 + 20, area->y1, area->y2 + 1, color_p);
  }
  screen::screen(/* args */) : spi(320, [](void *data) {})
  {
    _instance = this;
    spi.write(1, 10, 2, 10, (void*)"123456");
    lv_init();
    /*lcd接口初始化*/
    // lcd_init();

    /* 注册显示驱动 */
    lv_display_init();

    /*触摸芯片初始化*/
    // tp_init();

    /* 注册输入驱动*/
    lv_port_indev_init();

    /* 初始化LVGL定时器 */
    lv_port_tick_init();
  }

  void screen::flush(int x1, int y1, int x2, int y2, void *data)
  {
    // define an area of frame memory where MCU can access
  }

  void screen::lv_display_init(void)
  {
    static lv_disp_draw_buf_t draw_buf_dsc;
    size_t disp_buf_height = 40;

    /* 必须从内部RAM分配显存，这样刷新速度快 */
    void *p_disp_buf1 = heap_caps_malloc(LCD_WIDTH * disp_buf_height * sizeof(lv_color_t), MALLOC_CAP_INTERNAL | MALLOC_CAP_DMA);
    void *p_disp_buf2 = heap_caps_malloc(LCD_WIDTH * disp_buf_height * sizeof(lv_color_t), MALLOC_CAP_INTERNAL | MALLOC_CAP_DMA);
    ESP_LOGI(TAG, "Try allocate two %u * %u display buffer, size:%u Byte", LCD_WIDTH, disp_buf_height, LCD_WIDTH * disp_buf_height * sizeof(lv_color_t) * 2);
    if (NULL == p_disp_buf1 || NULL == p_disp_buf2)
    {
      ESP_LOGE(TAG, "No memory for LVGL display buffer");
      esp_system_abort("Memory allocation failed");
    }
    /* 初始化显示缓存 */
    lv_disp_draw_buf_init(&draw_buf_dsc, p_disp_buf1, p_disp_buf2, LCD_WIDTH * disp_buf_height);

    /* 初始化显示驱动 */
    lv_disp_drv_init(&disp_drv);

    /*设置水平和垂直宽度*/
    disp_drv.hor_res = LCD_WIDTH;  // 水平宽度
    disp_drv.ver_res = LCD_HEIGHT; // 垂直宽度

    // (*lambdaPtr)(disp_drv, area, color_p);
    /* 设置刷新数据函数 */
    static auto call_back = [](lv_disp_drv_t *disp_drv, const lv_area_t *area, lv_color_t *color_p)
    {
      ESP_LOGE(TAG, "screen::_instance->disp_flush(disp_drv, area, color_p);");
      screen::_instance->disp_flush(disp_drv, area, color_p);
    };
    disp_drv.flush_cb = call_back;

    /*设置显示缓存*/
    disp_drv.draw_buf = &draw_buf_dsc;

    /*注册显示驱动*/
    lv_disp_drv_register(&disp_drv);
  }

  void IRAM_ATTR indev_read(struct _lv_indev_drv_t *indev_drv, lv_indev_data_t *data)
  {
    // int16_t x, y;
    // int state;
    // cst816t_read(&x, &y, &state);
    // data->point.x = y;
    // if (x == 0)
    //   x = 1;
    // data->point.y = LCD_HEIGHT - x;
    // data->state = state;
  }
  screen::~screen()
  {
  }
}