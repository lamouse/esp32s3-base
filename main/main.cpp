/*
 * SPDX-FileCopyrightText: 2010-2022 Espressif Systems (Shanghai) CO LTD
 *
 * SPDX-License-Identifier: CC0-1.0
 */

#include <stdio.h>
#include <inttypes.h>
#include "sdkconfig.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_chip_info.h"
#include "esp_flash.h"
#include "esp_system.h"
#include "esp_log.h"
#include "screen.hpp"
#include "driver/gpio.h"
#include "wifi.hpp"
#include "DHT11.hpp"
#include "lvgl.h"
#include <widgets/lv_demo_widgets.h>
#include "lv_demos.h"
#include "key.hpp"
#include "QMI8658C.hpp"
#include "sd_card.hpp"
#include "microphone.hpp"
#include <fstream>
#include "audio.hpp"
#include "PCA9557.hpp"

static const char *TAG = "main";

void task(void *parameter)
{
    // gpio_set_direction(GPIO_NUM_48, GPIO_MODE_OUTPUT);
    // gpio_set_level(GPIO_NUM_48, 1);
    // sensor::DHT11 dht11(GPIO_NUM_2);
    // gpio_set_level(GPIO_NUM_0,0);
    // display::screen screen;
    // lv_demo_widgets();
    // vTaskDelay(pdMS_TO_TICKS(1000));
    hardware::audio audio;
    hardware::PCA9557 pca;
    pca.set_state(PA_EN_GPIO, 1);
    hardware::key k(GPIO_NUM_0);
    // sensor::QMI8658C qmi(I2C_NUM_0, GPIO_NUM_1, GPIO_NUM_2, 400 * 1000);
    // for(int i = 0; i < 3; i++){
    //     try
    //     {
    //         vTaskDelay(pdMS_TO_TICKS(500));
    //         auto qmi_data = qmi.read();
    //         ESP_LOGI(TAG, "test angle_x = %.1f  angle_y = %.1f angle_z = %.1f", qmi_data.x, qmi_data.y, qmi_data.z);
    //     }
    //     catch (const std::exception &e)
    //     {
    //         ESP_LOGE(TAG, "QMI8658C init err: %s", e.what());
    //         ESP_LOGE(TAG, "restrat system....");
    //         esp_restart();
    //     }
    // }

    while (1)
    {
        vTaskDelay(pdMS_TO_TICKS(1000));
        // auto [t, h] = dht11.get();
        // printf("----t = %f, s=%d\n", float(t) / 10, h);
        auto state = k.get_state();
        switch (state)
        {
        case hardware::key_stat::no_action:
            break;
        case hardware::key_stat::put_down:
            printf("----key put down\n");
            break;
        case hardware::key_stat::put_up:
            printf("----key put up\n");

            audio.i2s_music(nullptr);
            break;
        }
        try
        {
            // auto qmi_data = qmi.read();
            // ESP_LOGI(TAG, "angle_x = %.1f  angle_y = %.1f angle_z = %.1f", qmi_data.x, qmi_data.y, qmi_data.z);
        }
        catch (const std::exception &e)
        {
            ESP_LOGE(TAG, "QMI8658C err: %s", e.what());
        }
    }
    // lv_task_handler();
}

extern "C" void app_main(void)
{
    device::wifi wifi("showmeyourbp", "WW6639270");
    // hardware::sd_card sd(GPIO_NUM_47, GPIO_NUM_48, GPIO_NUM_21);
    // hardware::microphone mic;

    task(nullptr);
    // xTaskCreatePinnedToCore(task, "test task", 2048, nullptr, 3, nullptr, 0);
}
