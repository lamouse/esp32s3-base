#include "camera.hpp"
#include "esp_log.h"
#include "PCA9557.hpp"
static const char *CAMERA_TAG = "audio";
namespace hardware
{

    camera::camera(/* args */)
    {
        hardware::PCA9557 pca;

        pca.set_state(DVP_PWDN_GPIO, 0);
        camera_config_t config;
        config.ledc_channel = LEDC_CHANNEL_1; // LEDC通道选择  用于生成XCLK时钟 但是S3不用
        config.ledc_timer = LEDC_TIMER_1;     // LEDC timer选择  用于生成XCLK时钟 但是S3不用
        config.pin_d0 = CAMERA_PIN_D0;
        config.pin_d1 = CAMERA_PIN_D1;
        config.pin_d2 = CAMERA_PIN_D2;
        config.pin_d3 = CAMERA_PIN_D3;
        config.pin_d4 = CAMERA_PIN_D4;
        config.pin_d5 = CAMERA_PIN_D5;
        config.pin_d6 = CAMERA_PIN_D6;
        config.pin_d7 = CAMERA_PIN_D7;
        config.pin_xclk = CAMERA_PIN_XCLK;
        config.pin_pclk = CAMERA_PIN_PCLK;
        config.pin_vsync = CAMERA_PIN_VSYNC;
        config.pin_href = CAMERA_PIN_HREF;
        config.pin_sccb_sda = -1; // 这里写-1 表示使用已经初始化的I2C接口
        config.pin_sccb_scl = CAMERA_PIN_SIOC;
        config.sccb_i2c_port = 0;
        config.pin_pwdn = CAMERA_PIN_PWDN;
        config.pin_reset = CAMERA_PIN_RESET;
        config.xclk_freq_hz = XCLK_FREQ_HZ;
        config.pixel_format = PIXFORMAT_RGB565;
        config.frame_size = FRAMESIZE_QVGA;
        config.jpeg_quality = 12;
        config.fb_count = 2;
        config.fb_location = CAMERA_FB_IN_PSRAM;
        config.grab_mode = CAMERA_GRAB_WHEN_EMPTY;

        // camera init
        esp_err_t err = esp_camera_init(&config); // 配置上面定义的参数
        if (err != ESP_OK)
        {
            ESP_LOGE(CAMERA_TAG, "Camera init failed with error 0x%x", err);
            return;
        }

        sensor_t *s = esp_camera_sensor_get(); // 获取摄像头型号

        if (s->id.PID == GC0308_PID)
        {
            s->set_hmirror(s, 1); // 这里控制摄像头镜像 写1镜像 写0不镜像
        }
    }

    void  camera::frame_process(frame_process_fun fun){
        auto frame = std::make_shared<camera_frame>();
        fun(frame);
    }

    camera::~camera()
    {
    }

}