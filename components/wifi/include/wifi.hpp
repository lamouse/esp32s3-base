#ifndef WIFI_HPP
#define WIFI_HPP
#include "esp_wifi.h"
#include <string>
#include <vector>
#include <functional>

namespace device{
    using wifi_event_id = int32_t;
    using wifi_connect_callback_t = std::function<void(wifi_event_id)>;
    class  wifi
    {
    private:
        ::std::string SSID;
        ::std::string password;
    public:
         wifi();
         std::vector<wifi_ap_record_t> scan(void);
         void connect(std::string SSID, std::string password, wifi_connect_callback_t cb,
                wifi_auth_mode_t auth_mode = WIFI_AUTH_WPA2_PSK);
        ~ wifi();
    };
}

#endif