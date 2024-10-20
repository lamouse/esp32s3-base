#ifndef SCREEN_HPP
#define SCREEN_HPP
#include "spi.hpp"
#include "lvgl.h"


namespace display{

    class screen
    {
    private:
        driver::SPI spi;
        void lv_display_init(void);

    public:
        screen();
        void flush(int x1, int y1, int x2, int y2, void* data);
        ~screen();
    };


}

#endif