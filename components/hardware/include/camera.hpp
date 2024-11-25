#pragma once
#include "esp_camera.h"
#include "functional"
#include <memory>

/***********************************************************/
/****************    摄像头 ↓   ****************************/
#define CAMERA_PIN_PWDN -1
#define CAMERA_PIN_RESET -1
#define CAMERA_PIN_XCLK 5
#define CAMERA_PIN_SIOD 1
#define CAMERA_PIN_SIOC 2

#define CAMERA_PIN_D7 9
#define CAMERA_PIN_D6 4
#define CAMERA_PIN_D5 6
#define CAMERA_PIN_D4 15
#define CAMERA_PIN_D3 17
#define CAMERA_PIN_D2 8
#define CAMERA_PIN_D1 18
#define CAMERA_PIN_D0 16
#define CAMERA_PIN_VSYNC 3
#define CAMERA_PIN_HREF 46
#define CAMERA_PIN_PCLK 7

#define XCLK_FREQ_HZ 24000000
namespace hardware
{

    class camera
    {
    private:
        /* data */
    public:
        class camera_frame
        {
        private:
            camera_fb_t *_frame;

        public:
            camera_fb_t *get() { return _frame; }
            camera_frame()
            {
                _frame = esp_camera_fb_get();
            };

            ~camera_frame()
            {
                esp_camera_fb_return(_frame);
            }
        };
        using frame_process_fun = std::function<void(std::shared_ptr<camera_frame>)>;

        camera(/* args */);
        void frame_process(frame_process_fun fun);
        ~camera();
    };
}