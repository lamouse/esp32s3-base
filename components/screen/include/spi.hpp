#ifndef SPI_HPP
#define SPI_HPP
#include <cstddef>
#include <cstdint>
#include <functional>
#include <esp_lcd_panel_io.h>

/**
 * @brief SPI driver
 * 
 */

namespace driver {
    using lcd_flush_callback_t = std::function<void(void*)>;
    class SPI {
        public:
            SPI(uint16_t width, lcd_flush_callback_t callback);
            ~SPI();
            void write(int x1, int y1, int x2, int y2, void* data);
            void read(uint8_t *data, size_t len);
            void transfer(uint8_t *data, size_t len);
        private:
            void set_speed(uint32_t speed);
            void set_mode(uint8_t mode);
            esp_lcd_panel_io_handle_t lcd_io_handle = NULL;
    
};
} 

#endif
