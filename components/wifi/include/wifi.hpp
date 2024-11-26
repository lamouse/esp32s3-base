#ifndef WIFI_HPP
#define WIFI_HPP
#include "esp_wifi.h"
#include <string>
#include <vector>

namespace device{
    class  wifi
    {
    private:
        ::std::string SSID;
        ::std::string password;
    public:
         wifi();
         std::vector<wifi_ap_record_t> scan(void);
        ~ wifi();
    };
}

#endif