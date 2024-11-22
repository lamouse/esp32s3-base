#ifndef QMI8658C_HPP
#define QMI8658C_HPP
#include "driver/i2c.h"
#include <string>
#include <vector>

namespace sensor{
    class QMI8658C
    {
    private:
        std::vector<uint8_t> register_read(uint8_t reg_addr, int size);
        void register_write(uint8_t reg_addr, uint8_t data);
        void register_read(uint8_t reg_addr, uint8_t *data, size_t len);
        i2c_port_t _i2c_port;
    public:
        struct angle
        {
            float x;
            float y;
            float z;
        };

        class ReadException : public std::exception{
            public:
                const char* what() const noexcept override {
                    return "read QMI8658C data exception";
                }
        };

        QMI8658C(i2c_port_t i2c_port, int sda, int scl, uint32_t scl_speed);
        angle read();
        ~QMI8658C();
    };

}
#endif
