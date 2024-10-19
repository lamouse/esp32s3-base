#define LV_USE_PRIVATE_API 1
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_err.h"
#include "esp_log.h"
#include "lvgl.h"
#include "screen.hpp"
#include "spi.hpp"


#define LCD_WIDTH   320
#define LCD_HEIGHT  240
static const char *TAG = "screen";
namespace display{
namespace{

}

  
screen::screen(/* args */): spi(320, [](void* data){})
{
  lv_init();
  
}


void screen::flush(int x1, int y1, int x2, int y2, void* data){
  // define an area of frame memory where MCU can access
 
}

void screen::lv_display_init(void){
    size_t disp_buf_height = 40;
      /* 必须从内部RAM分配显存，这样刷新速度快 */
    void *p_disp_buf1 = heap_caps_malloc(LCD_WIDTH * disp_buf_height * sizeof(lv_color_t), MALLOC_CAP_INTERNAL | MALLOC_CAP_DMA);
    void *p_disp_buf2 = heap_caps_malloc(LCD_WIDTH * disp_buf_height * sizeof(lv_color_t), MALLOC_CAP_INTERNAL | MALLOC_CAP_DMA);
    ESP_LOGI(TAG, "Try allocate two %u * %u display buffer, size:%u Byte", LCD_WIDTH, disp_buf_height, LCD_WIDTH * disp_buf_height * sizeof(lv_color_t) * 2);
    if (NULL == p_disp_buf1 || NULL == p_disp_buf2) {
        ESP_LOGE(TAG, "No memory for LVGL display buffer");
        esp_system_abort("Memory allocation failed");
    }
    lv_display_create()
    
}

screen::~screen()
{
}
}