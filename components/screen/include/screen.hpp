#ifndef SCREEN_HPP
#define SCREEN_HPP
#include "spi.hpp"
#include "lvgl.h"


namespace display{

    class screen
    {
    private:
        driver::SPI spi;
        static screen* _instance;
        void lv_display_init(void);
        void disp_flush(lv_disp_drv_t *disp_drv, const lv_area_t *area, lv_color_t *color_p);
    public:
        screen();
        void flush(int x1, int y1, int x2, int y2, void* data);
        ~screen();
    };


}

#endif