/*
 * SPDX-FileCopyrightText: 2010-2022 Espressif Systems (Shanghai) CO LTD
 *
 * SPDX-License-Identifier: CC0-1.0
 */

#include "DHT11.hpp"
#include "PCA9557.hpp"
#include "QMI8658C.hpp"
#include "audio.hpp"
#include "camera.hpp"
#include "demos/lv_demos.h"
#include "driver/gpio.h"
#include "esp_chip_info.h"
#include "esp_flash.h"
#include "esp_lcd_panel_ops.h"
#include "esp_log.h"
#include "esp_system.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "key.hpp"
#include "lv_demos.h"
#include "lvgl.h"
#include "microphone.hpp"
#include "screen.hpp"
#include "sd_card.hpp"
#include "sdkconfig.h"
#include "system.hpp"
#include "wifi.hpp"
#include "wifi_app.hpp"
#include "yingwu.h"
#include <fstream>
#include <inttypes.h>
#include <stdio.h>
//#include <widgets/lv_demo_widgets.h>
#include "ui.h"

static const char *TAG = "main";
// 定义lcd显示队列句柄
static QueueHandle_t xQueueLCDFrame = NULL;
// lcd处理任务
static void task_process_lcd(void *arg) {
  std::shared_ptr<hardware::camera::camera_frame> frame_ptr;
  display::screen *scren = static_cast<display::screen *>(arg);
  while (true) {
    if (xQueueReceive(xQueueLCDFrame, &frame_ptr, portMAX_DELAY)) {
      auto frame = frame_ptr;
      if (frame_ptr->get()) {
        scren->draw_bitmap(0, 0, frame_ptr->get()->width,
                           frame_ptr->get()->height,
                           (uint16_t *)frame_ptr->get()->buf);
      }
    }
  }
}

// 摄像头处理任务
static void task_process_camera(void *arg) {

  hardware::camera *camera = static_cast<hardware::camera *>(arg);

  while (true) {
    camera->frame_process([](auto d) {
      long t = d.use_count();
      xQueueSend(xQueueLCDFrame, &d, portMAX_DELAY);
      while (d.use_count() == t)
        ;
    });
  }
}

void task(void *parameter) {
  // gpio_set_direction(GPIO_NUM_48, GPIO_MODE_OUTPUT);
  // gpio_set_level(GPIO_NUM_48, 1);
  // sensor::DHT11 dht11(GPIO_NUM_2);
  // gpio_set_level(GPIO_NUM_0,0);
  // display::screen screen;
  // lv_demo_widgets();
  // vTaskDelay(pdMS_TO_TICKS(1000));
  // hardware::audio audio;
  // hardware::PCA9557 pca;
  // pca.set_state(PA_EN_GPIO, 1);
  hardware::key k(GPIO_NUM_0);
  // sensor::QMI8658C qmi(I2C_NUM_0, GPIO_NUM_1, GPIO_NUM_2, 400 * 1000);
  // for(int i = 0; i < 3; i++){
  //     try
  //     {
  //         vTaskDelay(pdMS_TO_TICKS(500));
  //         auto qmi_data = qmi.read();
  //         ESP_LOGI(TAG, "test angle_x = %.1f  angle_y = %.1f angle_z = %.1f",
  //         qmi_data.x, qmi_data.y, qmi_data.z);
  //     }
  //     catch (const std::exception &e)
  //     {
  //         ESP_LOGE(TAG, "QMI8658C init err: %s", e.what());
  //         ESP_LOGE(TAG, "restrat system....");
  //         esp_restart();
  //     }
  // }

  while (1) {
    vTaskDelay(pdMS_TO_TICKS(1000));
    // auto [t, h] = dht11.get();
    // printf("----t = %f, s=%d\n", float(t) / 10, h);
    auto state = k.get_state();
    switch (state) {
    case hardware::key_stat::no_action:
      break;
    case hardware::key_stat::put_down:
      printf("----key put down\n");
      break;
    case hardware::key_stat::put_up:
      printf("----key put up\n");

      // audio.i2s_music(nullptr);
      break;
    }
    try {
      // auto qmi_data = qmi.read();
      // ESP_LOGI(TAG, "angle_x = %.1f  angle_y = %.1f angle_z = %.1f",
      // qmi_data.x, qmi_data.y, qmi_data.z);
    } catch (const std::exception &e) {
      ESP_LOGE(TAG, "QMI8658C err: %s", e.what());
    }
  }
  // lv_task_handler();
}

extern "C" void app_main(void) {
  //  hardware::sd_card sd(GPIO_NUM_47, GPIO_NUM_48, GPIO_NUM_21);
  //  hardware::microphone mic;
  // app::init();
  display::screen scr;
  //app::app_wifi_connect();

  // scr.lcd_draw_pictrue(0, 0, 320, 240, gImage_yingwu); // 显示3只鹦鹉图片
  // vTaskDelay(pdMS_TO_TICKS(500));
  // app::app_wifi_connect();
  // hardware::camera camera;
  // xQueueLCDFrame = xQueueCreate(2,
  // sizeof(std::shared_ptr<hardware::camera::camera_frame>));
  // xTaskCreatePinnedToCore(task_process_camera, "task_process_camera", 3 *
  // 1024, &camera, 5, NULL, 1); xTaskCreatePinnedToCore(task_process_lcd,
  // "task_process_lcd", 4 * 1024, &scr, 5, NULL, 0);
  // lv_demo_benchmark();
  //lv_demo_widgets();
  ui_init();
  task(nullptr);
  // xTaskCreatePinnedToCore(task, "test task", 2048, nullptr, 3, nullptr, 0);
}
