#ifndef QMI8658C_HPP
#define QMI8658C_HPP

#include <string>
#include <vector>

namespace sensor{
    class QMI8658C
    {
    private:
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

        QMI8658C();
        angle read();
        ~QMI8658C();
    };

}
#endif
